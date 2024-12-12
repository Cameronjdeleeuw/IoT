import paho.mqtt.client as mqtt
import mysql.connector

# MQTT Broker Configuration
BROKER_URL = "broker.mqtt-dashboard.com"
BROKER_PORT = 1883
TOPIC = "testtopic/temp/outTopic/updatePotRead1883"  # Topic to subscribe to

# MySQL Database Configuration
HOST = "srv1614.hstgr.io"  # Hostinger database hostname
USER = "u413365777_db_camerondele"  # Database username
PASSWORD = "EEssu2024"  # Replace with your actual password
DATABASE = "u413365777_camerondeleeuw"  # Database name

# Function to insert data into the database
def push_value_to_db(node_name, pot_value):
    try:
        # Connect to the database
        connection = mysql.connector.connect(
            host=HOST,
            user=USER,
            password=PASSWORD,
            database=DATABASE
        )

        if connection.is_connected():
            print("Connected to the database!")

            # Create a cursor object
            cursor = connection.cursor()

            # SQL query to insert data
            insert_query = "INSERT INTO potentiometer_data (node_name, pot_value) VALUES (%s, %s)"
            cursor.execute(insert_query, (node_name, pot_value))

            # Commit the transaction
            connection.commit()
            print(f"Node '{node_name}' with value {pot_value} inserted into the database!")

    except mysql.connector.Error as err:
        print(f"Database error: {err}")
    finally:
        if connection.is_connected():
            cursor.close()
            connection.close()
            print("Database connection closed.")

# MQTT Callback for when a message is received
def on_message(client, userdata, msg):
    try:
        pot_value = int(msg.payload.decode())  # Convert the received payload to an integer
        print(f"Received potentiometer value: {pot_value}")
        push_value_to_db("node1", pot_value)  # Insert the value into the database
    except ValueError:
        print(f"Invalid data received: {msg.payload.decode()}")

# MQTT Callback for connection
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT broker!")
        client.subscribe(TOPIC)
        print(f"Subscribed to topic: {TOPIC}")
    else:
        print(f"Failed to connect, return code {rc}")

# Set up the MQTT client
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

# Connect to the MQTT broker
print("Connecting to MQTT broker...")
client.connect(BROKER_URL, BROKER_PORT, 60)

# Start the MQTT client loop
try:
    client.loop_forever()
except KeyboardInterrupt:
    print("MQTT client disconnected.")
