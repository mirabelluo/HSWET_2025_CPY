from matplotlib import pyplot as plt
import pandas as pd
import os as os 
from pathlib import Path
import numpy as np

out_folder_path = "HSWET/Figures/"
os.makedirs(out_folder_path, exist_ok= True)
#Give input file path
file_path = "HSWET/RPM_vs_Time_test.csv" 

#set date:
#date = "04-24-2025"
#os.makedirs(os.path.join(out_folder_path, date +"_Tests"), exist_ok= True)

#out_folder_path = out_folder_path + date + "_Tests/"

#read CSV
df = pd.read_csv(file_path)

#assume that time between data logging is 0.25 seconds
time = np.arange(0, len(df) * 0.25, 0.25)
rpm = df[' RPM'].values

fig = plt.figure(figsize=(8, 6))
plt.plot(time, rpm)
plt.xlabel('Time (s)', fontsize = 20, family = 'Arial')
plt.ylabel('RPM (rpm)', fontsize = 20, family = 'Arial')
plt.title('RPM vs Time', fontsize = 20, family = 'Arial')
plt.grid(True, color = "lightgray", alpha = 0.5)
plt.savefig(out_folder_path + "RPM_v_Time.png") #specify path to store data
plt.close(fig)

# TODO given multiple inputs Power ratio versus time versus rpm ratio 
