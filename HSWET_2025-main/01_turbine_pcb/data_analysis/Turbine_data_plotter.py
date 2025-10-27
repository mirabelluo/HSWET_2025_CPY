import pandas as pd
import os as os
from matplotlib import pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import numpy as np
from pathlib import Path
from scipy.interpolate import griddata

# TODO make fonts bigger
 
#Change "HSWET/Figures" for different output folder path 
out_folder_path = "HSWET/Figures/"
os.makedirs(out_folder_path, exist_ok= True)
#Change inside Path -> "Windspeed Tests Outputs" for a different input folder path to loop through
folder_path = Path("HSWET/Windspeed_Tests_Outputs")
os.makedirs(folder_path, exist_ok= True) 
#set date:
date = "04-24-2025"
os.makedirs(os.path.join(out_folder_path, date +"_Tests/"), exist_ok= True)

out_folder_path = out_folder_path + date + "_Tests/"

#create new dataframe to hold all max data
maxes_csv = pd.DataFrame(columns = ['Windspeed', 'Pitch', 'Voltage', 'Current', 'Resistance', 
                                    'Power', 'RPM', 'Load Setting'])

#iterate through all files in folder
for file_path in folder_path.glob("*.csv"):

    df = pd.read_csv(file_path)
    length = len(file_path.name)
    out_file_path = (file_path.name)[:length - 4]

    #add the row with max power to output dataframe
    max_power_row = df['Power'].idxmax()    
    maxes_csv = pd.concat([maxes_csv, df.loc[[max_power_row]]], ignore_index = True)

    #define grid range
    pitch_vals = np.linspace(df['Pitch'].min(), df['Pitch'].max(), 50)
    resistance_vals = np.linspace(df['Resistance'].min(), df['Resistance'].max(), 50)
    Pitch_grid, Resistance_grid = np.meshgrid(pitch_vals, resistance_vals)

    #interpolate Power values on the grid
    Power_grid = griddata(
        (df['Pitch'], df['Resistance']),
        df['Power'],
        (Pitch_grid, Resistance_grid),
        method='cubic'
    )

    #create the 3D surface plot
    fig = plt.figure(figsize=(12, 8))
    ax = fig.add_subplot(111, projection='3d')
    surf = ax.plot_surface(Pitch_grid, Resistance_grid, Power_grid, cmap='viridis')
    ax.set_xlabel('Pitch', fontsize = 20, family = 'Arial')
    ax.set_ylabel('Resistance (Ohms)', fontsize = 20, family = 'Arial')
    ax.set_zlabel('Power (W)', fontsize = 20, family = 'Arial')
    fig.colorbar(surf, ax=ax, label='Power (W)')
    ax.set_title('3D Surface Plot: Power as a Function of Pitch and Resistance', fontsize = 20, family = 'Arial')
    light_gray = "#DDDDDD"
    ax.xaxis._axinfo['grid']['color'] = light_gray
    ax.yaxis._axinfo['grid']['color'] = light_gray
    ax.zaxis._axinfo['grid']['color'] = light_gray
    plt.savefig(out_folder_path + out_file_path + "1.png") #change "Figure/" to specify output path
    plt.close(fig)


    # Option 2: 2D Plot - Power vs Resistance for different Pitch values
    fig, ax = plt.subplots(figsize=(10, 8))
    for pitch_value in sorted(df['Pitch'].unique()):
        subset = df[df['Pitch'] == pitch_value]
        ax.plot(subset['Resistance'], subset['Power'], label=f'Pitch {pitch_value}')
    ax.set_xlabel('Resistance (Ohms)', fontsize = 20, family = 'Arial')
    ax.set_ylabel('Power (W)', fontsize = 20, family = 'Arial')
    ax.set_title('Power vs Resistance for Different Pitch Values', fontsize = 20, family = 'Arial')
    ax.legend(fontsize = 14)
    plt.grid(True, color = "lightgray", alpha = 0.5)
    plt.savefig(out_folder_path + out_file_path + "2.png") #specify path to store data
    plt.close(fig)


    # Option 3: 2D Plot - Power vs Pitch for different Resistance values
    fig, ax = plt.subplots(figsize=(10, 8))
    for resistance_value in sorted(df['Resistance'].unique()):
        subset = df[df['Resistance'] == resistance_value]
        ax.plot(subset['Pitch'], subset['Power'], label=f'Resistance {resistance_value}')
    ax.set_xlabel('Pitch', fontsize = 20, family = 'Arial')
    ax.set_ylabel('Power (W)', fontsize = 20, family = 'Arial')
    ax.set_title('Power vs Pitch for Different Resistance Values', fontsize = 20, family = 'Arial')
    ax.legend(fontsize = 14)
    plt.grid(True, color = "lightgray", alpha = 0.5)
    plt.savefig(out_folder_path + out_file_path +  "3.png") #specify path to store data
    plt.close(fig)

maxes_csv.to_csv(out_folder_path + "Maxes.csv", index = False) #output/save maxes csv
