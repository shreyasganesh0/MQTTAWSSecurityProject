import os, time, json, boto3, logging
from decimal import Decimal

logger = logging.getLogger()
logger.setLevel(logging.INFO)

# Environment configuration
VERIFIED_TABLE = os.environ['VERIFIED_TABLE']
SENSOR_TABLE   = os.environ['SENSOR_TABLE']
BANNED_TABLE   = os.environ['BANNED_TABLE']
CHECK_INTERVAL = int(os.environ.get('DURATION', '3600'))
LOG_GROUP      = os.environ['LOG_GROUP']

# AWS resource/clients
dynamodb       = boto3.resource('dynamodb')
verified_table = dynamodb.Table(VERIFIED_TABLE)
sensor_table   = dynamodb.Table(SENSOR_TABLE)
banned_table   = dynamodb.Table(BANNED_TABLE)
logs           = boto3.client('logs')
iot            = boto3.client('iot')


ok_cache = {}

def get_source_port(device, ip):
    """
    Query CloudWatch connect logs for the most recent
    connect event matching deviceId & ipAddress, return sourcePort.
    """
    start_ms   = int((time.time() - CHECK_INTERVAL) * 1000)
    filter_pat = f'{{ $.clientId = "{device}" && $.ipAddress = "{ip}" }}'
    resp = logs.filter_log_events(
        logGroupName=LOG_GROUP,
        startTime=start_ms,
        filterPattern=filter_pat,
        limit=1
    )                                                           
    events = resp.get('events', [])
    if not events:
        return None
    msg = json.loads(events[0]['message'])
    return msg.get('sourcePort')

def purge_cache():
    """Remove cache entries older than CHECK_INTERVAL."""
    cutoff = time.time() - CHECK_INTERVAL
    for k, v in list(ok_cache.items()):
        if v < cutoff:
            del ok_cache[k]

def lambda_handler(event, context):
    """
    1) Extract deviceId, ipAddr, ts, temperature, humidity.
    2) Get sourcePort from CloudWatch.
    3) Fast‑path if in ok_cache.
    4) Else verify against DynamoDB.
    5) On success: cache & store sensor data.
    6) On failure: ban device & log to BannedDevices.
    """
    device   = event.get('deviceId')
    ip       = event.get('ipAddr')
    ts       = event.get('ts', int(time.time()))
    temp     = event.get('temperature')
    hum      = event.get('humidity')
    now      = time.time()

    if not all([device, ip, temp is not None, hum is not None]):
        logger.error("Malformed event: %s", event)
        return

    # 1) Cleanup cache
    purge_cache()

    # 2) Lookup port
    port = get_source_port(device, ip)
    if port is None:
        logger.warning("No port for %s@%s; banning", device, ip)
        port = -1

    key = f"{device}|{ip}|{port}"

    # 3) Fast‑path verification
    if key in ok_cache:
        ok_cache[key] = now
        logger.info("Cache hit; storing data for %s", key)
        sensor_table.put_item(
            Item={
                'deviceId'   : device,
                'timestamp'  : int(ts),
                'ipAddr'     : ip,
                'port'       : int(port),
                'temperature': Decimal(str(temp)),
                'humidity'   : Decimal(str(hum))
            }
        )  
        return

    # 4) DynamoDB verification
    try:
        resp = verified_table.get_item(Key={'deviceId': device})
    except Exception as e:
        logger.error("DynamoDB get_item error: %s", e)
        resp = {}
    item = resp.get('Item')

    if item and item.get('ipAddr') == ip and int(item.get('port', -1)) == port:
        # Verified – cache & store
        ok_cache[key] = now
        logger.info("Verified %s; storing data", key)
        sensor_table.put_item(
            Item={
                'deviceId'   : device,
                'timestamp'  : int(ts),
                'ipAddr'     : ip,
                'port'       : int(port),
                'temperature': Decimal(str(temp)),
                'humidity'   : Decimal(str(hum))
            }
        )  
        # Optionally update lastChecked
        verified_table.update_item(
            Key={'deviceId': device},
            UpdateExpression="SET lastChecked = :t",
            ExpressionAttributeValues={':t': int(now)}
        )
        return

    # 5) Ban device: detach & deactivate certificates
    logger.warning("Banning %s at %s:%s", device, ip, port)
    # 5a) Log ban to DynamoDB
    banned_table.put_item(
        Item={
            'deviceId'   : device,
            'ipAddr'     : ip,
            'port'       : int(port),
            'bannedAt'   : int(now)
        }
    )  
    # 5b) Revoke credentials
    try:
        principals = iot.list_thing_principals(thingName=device)['principals']
    except iot.exceptions.ResourceNotFoundException:
        principals = []
    for arn in principals:
        iot.detach_thing_principal(
            thingName=device,
            principal=arn
        )  
        cert_id = arn.rsplit('/',1)[-1]
        iot.update_certificate(
            certificateId=cert_id,
            newStatus='INACTIVE'
        )  
        logger.info("Revoked cert %s for %s", cert_id, device)

