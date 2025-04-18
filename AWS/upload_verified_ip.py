import os
import time
import base64
import boto3

# Environment variable
TABLE_NAME = os.environ['TABLE_NAME']  

# AWS resources
dynamodb = boto3.resource('dynamodb')
table    = dynamodb.Table(TABLE_NAME)

def lambda_handler(event, context):
    """
    Event from IoT Rule:
      { "b64": "<base64 payload>", "ipAddr": "203.0.113.5" }
    """
    b64     = event.get('b64')
    ip_addr = event.get('ipAddr')
    if not b64 or not ip_addr:
        return  # malformed event

    # Decode original message, e.g. "002:OK:192.168.1.23:5678"
    raw = base64.b64decode(b64).decode('utf-8')
    parts = raw.split(':')
    if len(parts) != 4 or parts[1] != 'OK':
        return  # unexpected format

    device_id = parts[0]
    ip_field  = parts[2]
    port      = parts[3]
    ts        = int(time.time())

    # Write to DynamoDB
    table.put_item(
        Item={
            'deviceId' : event.get('deviceId'),
            'timestamp': ts,
            'ipAddr'   : ip_field,
            'port'     : port
        }
    )

