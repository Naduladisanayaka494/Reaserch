import matplotlib
matplotlib.use('agg')  # Use the 'agg' backend
import numpy as np
from flask import Flask, request, jsonify
from keras.models import load_model
import joblib
import numpy as np
import matplotlib.pyplot as plt
import numpy as np
import warnings
from sklearn.exceptions import DataConversionWarning
warnings.filterwarnings(action='ignore', category=DataConversionWarning)

app = Flask(__name__)

# Load the trained model
model = load_model('Parkinsons_Trained_model.h5')

# Load the StandardScaler
scaler = joblib.load('scaler.pkl')

data_buffer = {}
recNo = 0
dataReady = False
tremor_score = 0
rigidity_score = 0
bradykinesia_score = 0
Postural_Instability_Score = 0

def convert_to_new_range(value):
    if value >= 10 and value <= 53:
        return (value - 10) * (1.9 - 1) / (53 - 10) + 1
    elif value > 53 and value <= 97:
        return (value - 54) * (2.9 - 2) / (97 - 54) + 2
    elif value > 97 and value <= 141:
        return (value - 98) * (3.9 - 3) / (141 - 98) + 3
    elif value > 141 and value <= 185:
        return (value - 142) * (4.9 - 4) / (185 - 142) + 4
    elif value > 185 and value <= 260:
        return (value - 186) * (5.9 - 5) / (260 - 186) + 5
    else:
        return 0

def tremorScore(data):
    RateRollArray = data['RateRollArray']
    RatePitchArray = data['RatePitchArray']
    RateYawArray = data['RateYawArray']

    print('RateRollArray:',RateRollArray)
    print('RatePitchArray:',RatePitchArray)
    print('RateYawArray:',RateYawArray)

    meanRoll = sum(abs(x) for x in RateRollArray) / len(RateRollArray)
    meanPitch = sum(abs(x) for x in RatePitchArray) / len(RatePitchArray)
    meanYaw = sum(abs(x) for x in RateYawArray) / len(RateYawArray)

    print('meanRoll:',meanRoll)
    print('meanPitch:',meanPitch)
    print('meanYaw:',meanYaw)

    return convert_to_new_range(meanPitch)

def rigidityScore(accel_data):
    # Calculate mean acceleration for each axis
    mean_accel_x = np.mean(accel_data['AccXArray'])
    mean_accel_y = np.mean(accel_data['AccYArray'])
    mean_accel_z = np.mean(accel_data['AccZArray'])
            
     # Calculate standard deviation of acceleration for each axis
    std_accel_x = np.std(accel_data['AccXArray'])
    std_accel_y = np.std(accel_data['AccYArray'])
    std_accel_z = np.std(accel_data['AccZArray'])
            
    # Calculate amplitude of acceleration for each axis
    amplitude_accel_x = np.max(accel_data['AccXArray']) - np.min(accel_data['AccXArray'])
    amplitude_accel_y = np.max(accel_data['AccYArray']) - np.min(accel_data['AccYArray'])
    amplitude_accel_z = np.max(accel_data['AccZArray']) - np.min(accel_data['AccZArray'])
            
    # Calculate rigidity score
    rigidity_score = (std_gyro_x+std_gyro_y+std_gyro_z)/3+(amplitude_gyro_x+amplitude_gyro_y+amplitude_gyro_z)/3
    #print("Rigidity Score: ",rigidity_score)  
     
    # Normalize rigidity score to a scale of 0-4
    
    normalized_rigidity_score = 4 - ((rigidity_score - 1.3) / (2.3 - 1.3)) * 4  # Assuming maximum possible value is 10
    if(normalized_rigidity_score<0):normalized_rigidity_score=0
    elif(normalized_rigidity_score>4):normalized_rigidity_score=4

    return normalized_rigidity_score

def bradykinesiaScore(accel_data):
    mean_accel_x = np.mean(accel_data['AccXArray'])
    mean_accel_y = np.mean(accel_data['AccYArray'])
    mean_accel_z = np.mean(accel_data['AccZArray'])
            
     # Calculate standard deviation of acceleration for each axis
    std_accel_x = np.std(accel_data['AccXArray'])
    std_accel_y = np.std(accel_data['AccYArray'])
    std_accel_z = np.std(accel_data['AccZArray'])
            
    # Calculate amplitude of acceleration for each axis
    amplitude_accel_x = np.max(accel_data['AccXArray']) - np.min(accel_data['AccXArray'])
    amplitude_accel_y = np.max(accel_data['AccYArray']) - np.min(accel_data['AccYArray'])
    amplitude_accel_z = np.max(accel_data['AccZArray']) - np.min(accel_data['AccZArray'])
            
    # Calculate rigidity score
    bradykinesia_score=(mean_heart_rate)/3+(std_heart_rate)/3  
    # Normalize rigidity score to a scale of 0-4
    normalized_bradykinesia_score = 4 - ((bradykinesia_score - 0.5) / (2 - 0.5)) * 4  # Assuming maximum possible value is 10
    print("normalized_bradykinesia Score: ",normalized_bradykinesia_score)  
    if(normalized_bradykinesia_score<0):normalized_bradykinesia_score=0
    elif(normalized_bradykinesia_score>4):normalized_bradykinesia_score=4

    #print("normalized_bradykinesia Score: ",normalized_bradykinesia_score)
    return normalized_bradykinesia_score

def PosturalInstabilityScore(gyro_data):
    mean_accel_x = np.mean(gyro_data['RateRollArray'])
    mean_accel_y = np.mean(gyro_data['RatePitchArray'])
    mean_accel_z = np.mean(gyro_data['RateYawArray'])
            
     # Calculate standard deviation of acceleration for each axis
    std_gyro_x = np.std(gyro_data['RateRollArray'])
    std_gyro_y = np.std(gyro_data['RatePitchArray'])
    std_gyro_z = np.std(gyro_data['RateYawArray'])
            
    # Calculate amplitude of acceleration for each axis
    amplitude_gyro_x = np.max(gyro_data['RateRollArray']) - np.min(gyro_data['RateRollArray'])
    amplitude_gyro_y = np.max(gyro_data['RatePitchArray']) - np.min(gyro_data['RatePitchArray'])
    amplitude_gyro_z = np.max(gyro_data['RateYawArray']) - np.min(gyro_data['RateYawArray'])
            
    # Calculate rigidity score
    Postural_Instability_Score = (std_gyro_x + std_gyro_y + std_gyro_z) / 3 + (amplitude_gyro_x + amplitude_gyro_y + amplitude_gyro_z) / 3
    #print("Postural_Instability_Score: ",Postural_Instability_Score)   
    # Normalize rigidity score to a scale of 0-4
    normalized_Postural_Instability_Score = ((Postural_Instability_Score - 66) / (200 - 66)) * 4 
    if(normalized_Postural_Instability_Score<0):normalized_Postural_Instability_Score=0
    elif(normalized_Postural_Instability_Score>4):normalized_Postural_Instability_Score=4

    #print("normalized_Postural_Instability_Score: ",normalized_Postural_Instability_Score)
    return normalized_Postural_Instability_Score


@app.route('/predict', methods=['POST'])
def predict():
    global tremor_score
    global rigidity_score
    global bradykinesia_score
    global Postural_Instability_Score
    global dataReady


tremor_score=(max_accel_x−min_accel_x) +(max_accel_y−min_accel_y)+(max_accel_z−min_accel_z)  / 3
​
    try:
        # Get the data from the request
        data = request.get_json()
        
        if(dataReady):
            print('Data array that come from the app : ', data)

            for i in [tremor_score, rigidity_score, bradykinesia_score, Postural_Instability_Score]:
                data['data'].append(str(i))

            print('Data array after adding device data : ', data)

            # Convert values
            gender_mapping = {'Male': 1, 'Female': 0}
            numerical_data = [float(value) if value.replace('.', '', 1).isdigit() else gender_mapping.get(value, value) for value in data['data']]

            # Convert numerical data to numpy array
            new_data_as_np = np.asarray(numerical_data)

            # Reshape the data to match the model input shape
            data_reshaped = new_data_as_np.reshape(1,-1)

            # Standardize the data
            std_data = scaler.transform(data_reshaped)

            # Make predictions
            prediction = model.predict(std_data)

            # Convert probabilities to class labels
            predicted_class = [np.argmax(prediction) + 1 for prediction in prediction]
            predicted_class = int(predicted_class[0])
            dataReady = False
            print('predicted_class', int(predicted_class))
            # Return the predicted class as JSON
            print(type(predicted_class))
            if (predicted_class == 1):
                return jsonify({'message':"Stage: 1"})
            elif (predicted_class == 2):
                return jsonify({'message': "Stage: 2"})
            elif (predicted_class == 3):
                return jsonify({'message': "Stage: 3"})
            elif (predicted_class == 4):
                print("success")
                return jsonify({'message': "Stage 4"})
            elif (predicted_class == 5):
                return jsonify({'message': "Stage: 5"})
        else:
            print('Data is not ready!')
            return jsonify({'error':"please complete the all tests using the device!"})

    except Exception as e:
        return jsonify({"error": str(e)})

    

@app.route('/process_data', methods=['POST'])
def process_data():
    global dataReady

    try:
        data = request.get_json()

        #print("data: ", data)
        if data is None:
            return jsonify({'error': 'Invalid JSON data'}), 400

        for key, value in data.items():
            if key == "recNo":
                recNo = int(value)
                print("recNo: ", recNo)
                continue

            data_buffer[key] = [float(val) for val in value.split(',')]

        #print(data_buffer)

        
        # Plotting accelerometer data
        plt.figure(figsize=(10, 6))
        plt.plot(data_buffer['AccXArray'], label='AccXArray')
        plt.plot(data_buffer['AccYArray'], label='AccYArray')
        plt.plot(data_buffer['AccZArray'], label='AccZArray')
        plt.title('Accelerometer Data')
        plt.xlabel('Time')
        plt.ylabel('Acceleration')
        plt.legend()
        plt.grid(True)
        plt.savefig('accelerometer.png')  # Save the plot instead of displaying
        plt.close()  # Close the plot to avoid memory leaks

        # Plotting Gyroscope data
        plt.figure(figsize=(10, 6))
        plt.plot(data_buffer['RateRollArray'], label='RateRollArray')
        plt.plot(data_buffer['RatePitchArray'], label='RatePitchArray')
        plt.plot(data_buffer['RateYawArray'], label='RateYawArray')
        plt.title('Gyroscope Data')
        plt.xlabel('Time')
        plt.ylabel('Gyro')
        plt.legend()
        plt.grid(True)
        plt.savefig('Gyroscope.png')  # Save the plot instead of displaying
        plt.close()  # Close the plot to avoid memory leaks

        #bradykinesiaScore(data_buffer)
        if(recNo == 1):
            # Tremor Score
            dataReady = False
            global tremor_score 
            tremor_score = tremorScore(data_buffer)
            print('TremorScore: ', tremor_score)
        elif(recNo == 2):
            global rigidity_score 
            rigidity_score = rigidityScore(data_buffer)
            print("Rigidity Score:", rigidity_score)
        elif(recNo == 3):
            # Bradykinesia Score
            global bradykinesia_score 
            bradykinesia_score = bradykinesiaScore(data_buffer)
            print('Bradykinesia Score : ', bradykinesia_score)
        elif(recNo == 4):
            # Postural Instability Score
            global Postural_Instability_Score 
            Postural_Instability_Score = PosturalInstabilityScore(data_buffer)
            print('Postural Instability Score : ', Postural_Instability_Score)
            dataReady = True
        
        return jsonify(data_buffer)

    except Exception as e:
        return jsonify({"error": str(e)})
    

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
