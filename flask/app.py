from flask import Flask, request, jsonify

app = Flask(__name__)

# Temporary list to store sensor data
sensor_data_list = []

@app.route('/sensor/data', methods=['POST'])
def post_sensor_data():
    # Get the data from the POST request
    data = request.get_json()
    temperature = data.get('temperature')
    humidity = data.get('humidity')
    
    # Add the data to the temporary list
    sensor_data = {
        'temperature': temperature,
        'humidity': humidity
    }
    sensor_data_list.append(sensor_data)
    
    # Return a success message
    return jsonify({'message': 'Data successfully received and stored'}), 200

@app.route('/sensor/data', methods=['GET'])
def get_sensor_data():
    # Return all the data in the temporary list
    return jsonify(sensor_data_list), 200

if __name__ == '__main__':
    app.run(debug=True)
