import os, base64, boto3, re

cwl   = boto3.client('logs')
iotd  = boto3.client('iot-data')

LOG_GROUP = os.environ['LOG_GROUP']
LAMBDA_ROLE = os.environ['ROLE_ARN']

def get_port(deviceId):
    # Query last Connect log for this clientId
    query = f"fields sourcePort | filter clientId = '{deviceId}' | sort @timestamp desc | limit 1"
    start   = int((time.time() - 300)*1000)
    resp    = cwl.start_query(logGroupName=LOG_GROUP,
                              startTime=start,endTime=int(time.time()*1000),
                              queryString=query)
    qid     = resp['queryId']
    # wait for results (simplified)
    result = cwl.get_query_results(queryId=qid)
    if result['results']:
        return result['results'][0][0]['value']  # sourcePort
    return None

def lambda_handler(event, context):
    b64      = event['b64']
    deviceId = event['deviceId']
    ip       = event['ip']
    raw      = base64.b64decode(b64).decode()
    port     = get_port(deviceId) or 'unknown'
    enriched = f"{raw}:{ip}:{port}"
    iotd.publish(topic='device/response/enriched', qos=1, payload=enriched)

