import os
import boto3
import logging

logger = logging.getLogger()
logger.setLevel(logging.INFO)

TABLE_NAME = os.environ['TABLE_NAME']
dynamodb = boto3.resource('dynamodb')
table = dynamodb.Table(TABLE_NAME)
iot = boto3.client('iot', region_name='us-east-2')

def lambda_handler(event, context):
    logger.info(f"Event: {event}")
    device = event.get('deviceId')
    if not device:
        logger.error("Missing deviceId in event")
        return

    # 1) Check DynamoDB for authorization record
    resp = table.get_item(Key={'deviceId': device})
    if 'Item' in resp:
        logger.info(f"{device} is authorized – no action taken")
        return

    # 2) List certificates attached to the Thing
    try:
        principals = iot.list_thing_principals(thingName=device).get('principals', [])
    except iot.exceptions.ResourceNotFoundException:
        logger.error(f"Thing {device} not found")
        return

    if not principals:
        logger.warning(f"No principals attached to {device}")
        return

    # 3) Detach & deactivate each cert so the device can no longer connect
    for cert_arn in principals:
        # Detach from Thing
        iot.detach_thing_principal(
            thingName=device,
            principal=cert_arn
        )
        # Deactivate certificate
        cert_id = cert_arn.rsplit('/', 1)[-1]
        iot.update_certificate(
            certificateId=cert_id,
            newStatus='INACTIVE'
        )
        logger.info(f"Deactivated cert {cert_id} for {device}")

