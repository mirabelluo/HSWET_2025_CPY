# HSWET_2025
# Tech inspection + testing 2:50 PM RUN SCRIPT #2 IF YOU CAN, GET SOME DATA!!!!
HSWET 2025 Code Implementations

Competition code: `06_CWC_2025` on the `main` branch.

# RUN STEP 2 FIRST

# Script #1: Verify the Variable Load
When we sweep through the r_lookup table with all 543 combinations, we want to ensure that all the measured resistances are actually correct.

i.e. we don't want to set 40ohms but end up getting 5ohms. That's not good.

So we'll sweep through all 543 combinations and verify the `set_resistance` and `measured_resistance`.

**Folder Structure:**
```
`step01_CWC_var_load` folder
    | `r_target.cpp`
    | `r_target.h`
    | `resistor_lookup.cpp`
    | `resistor_lookup.h`
    | `step01_rload_verification_sweep.ino`
```
**Step01: Create a New Sketch**
Add all the files to the same **Arduino sketch** (like how John does it), then run in Arduino IDE.

**Step02: Run the Arduino Code**
Run `step01_rload_verification_sweep.ino` 
 // This is a quick test to print the resistance values and their corresponding one-hot bitstrings
  // To make sure that all the resistors are working correctly
  // This function def is in r_target.cpp
  testAllResistors(0.5);

**Step03: Interpret the outputs**
If you see `set_R` and `measured_R` being very similar, and the `flag` is a mix of 0 and 1's (0 means OK, 1 means warning), then the full load is working properly.

At the end, the whole circuit should dieconnect via the `disconnectLoad()` function.

Here's a picture of what you expect to see:
![Turbine Test Image](https://media.discordapp.net/attachments/1296509803601854485/1371822830684999710/image.png?ex=68248912&is=68233792&hm=bf83fe3f162bc94227144308307e248fd0decf159c1fdf9815c89e05a42dfd63&=&format=webp&quality=lossless&width=1148&height=864)
Again, if you see a lot of `1`s, that's not good cuz there's lots of warnings. Some MOSFETS may be shorted/bad connections so you need to fix that.

# Script #2: Verify turbine health + collect data (Ask Shivam for help on this)
After the variable load is verified, the Aerodynamics team should put the nacelle together. 

## **Hardware:**
**Step01:  Attach the turbine dongle**
- 3 plugs in the front (9 signals total)
- 1 plug in the back (5 signals total). 
- All cable colors are matched, linear actuator BLUE goes to dongle BLACK.
- The **encoder** tends to get loose because they're just three female jumper wires rather than an actual cable. After tech inspection (no tape allowed), **you should probably add some duck tape to the center dongle**.
- Then, run the cable through the tower (ask Jacob), and connect Wilson's turbine cable duplicate #2.
- Now, the total length of this turbine cable (5 signals, black) should be **4 meters**, per competition requirement.
![Turbine Dongle](https://cdn.discordapp.com/attachments/1296509803601854485/1371840121011437689/IMG_6622.jpg?ex=6824992c&is=682347ac&hm=9daabb1277dbcf9384221503c03031d9a4685f7ce5c8e0570cd5cd5ddbc3aa69&)

- On the end of this cable, you should have a female end (no pins). Plug this cable into the **CTRL board** at the top, where you see **forked pin connectors**.
- Plug in the female cable with the shiny side facing down. Colors should match like this:
![Turbine Cable Connection](
https://cdn.discordapp.com/attachments/1296509803601854485/1371848629869871244/IMG_6778.png?ex=6824a119&is=68234f99&hm=6b1a31f8bb3f91478c940ecead8138f8ed9f15cf4e77e4299e4dc1484e9439d3&
)

**Step02: Attach the Power Cable**
Screw the red power cable under the screw terminals (positive) and (negative) of the rectifier. 
When Delancy shows up with the blue forked connector, **re-make this part with heat shrinks and blue forks and connect it properly**. 
![Turbine DC power cable](https://cdn.discordapp.com/attachments/1296509803601854485/1371851350769533030/image.png?ex=6824a3a1&is=68235221&hm=8d847c26e60d68408e18a3af130f3ad06fc4bea9eae504466b7f63a3fe6b48e6&)

Alex made this.
**[TODO AFTER INSPECTION]** 
![Competition Rules](https://cdn.discordapp.com/attachments/1296509803601854485/1371853046794944664/image.png?ex=6824a536&is=682353b6&hm=5bfe162cba646a28ee77607f2ac9bea7527ac773c5e5739e248d816e0b4aa985&)

Per competition rules, **the cable between turbine and CTRL box + LOAD box need to be Anderson PP** (the red and black blocks). 
![PCC Connection Diagram](https://cdn.discordapp.com/attachments/1296509803601854485/1371875220679294986/IMG_6789.jpg?ex=6824b9dc&is=6823685c&hm=6f74d9e761ff86be9a30d1b20643303918cbcf18db37b182b5a8ad8e0e39ed51&)


## **Software:** 

**Folder Structure**
`CWC_ctrl_box` folder
- `data_logged` folder: this keeps all the exp data (.csv files), including ones from yesterday's JHU tunnel tests.
- `step01_sweep_pitches_w_rload.ino`: upload this to the Arduino MEGA to find the best pitches and resistance values under each wind speed. Check [YouTube video demo on 05/12](https://youtu.be/YqWQHTz5Gho) (2.5 minutes long)

- To use the variable load, you have to add the following to the Arduino Sketch in **Arduino IDE**:
![Arduino header files for step02](https://cdn.discordapp.com/attachments/1296509803601854485/1371957899365974036/image.png?ex=682506dc&is=6823b55c&hm=e8f009f1d8a23d74f8300953bb4a0c1aa70ec6e4e7bc884fcccb1932b97fd86e&)

- `step02_low_pwr_curve_data.py`: You change the inputs in `step01_sweep_pitches_w_r_load.ino`, lines 9-16 (ask Shivam)
```
// [USER INPUT] Pitches and R-loads to sweep -------------------

// Resistive loads (ohm values)
int r_loads[] = {5, 10, 15, 20, 25, 30, 35};  
// Pitch positions (unit-less, full range is 1100 - 1500)
int positions[] = {1100, 1150, 1180, 1200, 1250, 1300, 1350, 1400, 1450}; 

// -------------------------------------------------------------
```
to change experiment conditions.

**Step01:** 
Open a new sketch, upload `step01_sweep_pitches_w_r_load.ino` with Arduino IDE.

**Step 02:**
CLOSE Arduino IDE, open VS Code, the `windwspeed`, `system_type`, and `port_name` on lines 8-20.
```
# ----------------------------[USER INPUT] -------------------------------
# At the test, you can ask the judge to set the speed to anything. 
# Say "can you set the wind speed to 10 m/s?" and the judge will set it to 10 m/s.
# So you change this value tp 10.00 and run the code.
# Unit: m/s
windspeed = 10

# Change to "Mac" if using Mac
system_type = "Windows"  

# Change to your Arduino's serial port
port_name = "COM6"  
# ------------------------------------------------------------------
```
**Step03:**
Run the code and see the `.csv` data saving.

**Step04:**
Tell the judge to set to a different windwpeed, change `windspeed` variable, and run again. 

**Step05:**
Run all the windspeeds with the judge, then analyze the data. 
Goal: find the highest powers under each windspeed, and see which blade_pitch and load resistances there are. Also, note the RPM values (important!!!)

This would be crucial for the actual competition tunnel tests.

# Script #3:
`step03_CWC5_automated_code.ino` 
## [TODO] 
## 1. Mirabel needs to add safety stuff
## 2. Wilson and Joaks need to add R-load settings - After you run Stript #2 first and analyzed the data to wee which pitch and R-loads are the best.

