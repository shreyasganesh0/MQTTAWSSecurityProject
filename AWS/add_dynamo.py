import os
import time
import boto3

# Read environment variables
TABLE_NAME      = os.environ['TABLE_NAME']
RESPONSE_WINDOW = int(os.environ.get('RESPONSE_WINDOW', '3600'))

# Initialize DynamoDB resource once (reused across invocations)
dynamodb = boto3.resource('dynamodb')
table    = dynamodb.Table(TABLE_NAME)

def lambda_handler(event, context):
    """
    TrackOK Lambda:
    Triggered by an AWS IoT rule on 'device/response'.
    Expects event = { "deviceId": "...", "ts": 1234567890 }
    Stores each deviceId with an expirationTime = now + RESPONSE_WINDOW.
    """
    # Extract deviceId from the rule
    device = event.get('deviceId')
    if not device:
        # Nothing to track if deviceId is missing
        return

    # Current UNIX time in seconds
    now = int(time.time())
    expiration = now + RESPONSE_WINDOW

    # Put item into DynamoDB with TTL attribute
    table.put_item(
        Item={
            'deviceId'      : device,
            'expirationTime': expiration
        }
    )

