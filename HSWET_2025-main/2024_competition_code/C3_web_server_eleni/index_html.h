const char PAGE_MAIN[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en" class="js-focus-visible">

<head>
  <script src="https://cdn.plot.ly/plotly-2.30.1.min.js" charset="utf-8"></script>
</head>

<title>HWSET WIFI Turbine Dashboard</title>

<style>

* {
  box-sizing: border-box;
  margin: 0;
  padding: 0;
}

body {
  font-family: Arial, sans-serif;
  background-color: #f0f0f0;
}

h3, button {
  font-family: Arial, sans-serif;
  font-size: 1rem;
}

h3 {
  margin: 0;
}


#title h3 {
  color: white;
  font-size: 1.25rem;
}


header {
  max-width: 96vw;
  margin: auto;
  border-radius: 0.5rem;
  background-color: #3498db;
  padding: 8px;
  text-align: center;

  display: flex;
  justify-content: space-between;
  align-items: center;
}

.stop-display-box {
  display: flex;
  align-items: center;
  justify-content: space-between; /* Flex items spaced apart */
  background-color: #f9f9f9;
  box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);
  border-radius: 5px;
  padding: 5px;

  padding-left: 10px;
  padding-right: 10px;
  gap: 0.8rem;

  width: 22%;
}

.stop-display-box h3 {
  margin: 0;
  padding: 5px;
}

.stop-display-box button {
  background-color: #3498db;
  font-weight: bold;
  color: #fff;
  border: none;
  border-radius: 5px;
  cursor: pointer;
  text-align: center;
  transition: background-color 0.4s ease;
  padding: 8px 16px;
  width: 50%;
}

.stop-btn {
  background-color: #dc847a !important;
}

.stop-btn:hover {
  background-color: #c13324 !important;
}

.run-btn {
  background-color: #a0b49e !important;
}

.run-btn:hover {
  background-color: #558451 !important;
}


main {
  max-width: 96vw;
  margin: 0.5rem auto;
  display: grid;
  grid-template-columns: 60% 39.5%; 
  gap: 0.5rem;
}

.left-display-box {
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: 3rem;
}

section {
  background-color: #fff;
  padding: 1rem;
  border-radius: 0.5rem;
  box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);
}

.charts-and-titles {
    display: grid;
    grid-template-columns: 73% 25%;
    column-gap: 2%;
    height: 78vh; /* 70% of viewport height */
    margin-bottom: 1rem;
}

.chart-box {
    width: 100%;
    height: 100%;
    background-color: #f0f0f0;
    border-radius: 10px;
    box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);
    grid-column: 1; /* Place in the first column (60%) */
    
}

.chart-text-column {
    display: flex;
    flex-direction: column;
    justify-content: space-between;
    height: 100%;
    grid-column: 2; /* Place in the first column (60%) */
}

.chart-textbox {
  display: flex;
  flex-direction: column;
  justify-content: center;
  align-items: center;
  height: calc(30% - 10px); /* 33% of the height with spacing */
  border-radius: 10px;
  box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);
}

.chart-textbox h3,
.chart-textbox p {
  margin: 0;
  text-align: center;
}

.chart-textbox h3 {
  font-size: 1rem;
  margin-bottom: 0.5rem;
}

.chart-textbox p {
  font-size: 2rem;
  font-weight: bold;
}


.chart{
  width: 100%;
  height: 100%; /* Use 100% height to fill the parent container */
}

/* title + data value */
.data-box {
    display: flex;
    justify-content: center; /* Center horizontally */
    align-items: center;
}

.data-box p {
  font-size:1rem;
  font-weight: bold;
}
    
.data-value {
  margin-left: 10px; /* Adjust margin as needed */
}

/* Left column bottom right, also used in radio buttons */
.data-display-box {
  justify-content: center; /* Center horizontally */
  align-items: center;
  background-color: #f9f9f9;
  padding: 2px;
  border-radius: 5px;
  display: grid;
  grid-template-columns: 1fr 1fr;
  box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);
}

.radio-group {
  display: flex;
  flex-direction: row;
  gap: 0.5rem;
  margin-top: 0.5rem;
  margin-bottom: 0.5rem;
}

.radio-group input[type="radio"] {
  transform: scale(1.2); /* Increase the size of the radio buttons */
}

.radio-display-box {
    display: flex;
    justify-content: center; /* Center horizontally */
    align-items: center;
    background-color: #f9f9f9;
    border-radius: 5px;
    flex: 1;
    gap: 0.5rem;
    
    padding-top: 0.5rem;
    padding-bottom: 0.5rem;
    margin-top: 0.3rem;
    box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);
}

/* Hover style for radio buttons */
.radio-display-box:hover {
  background-color: rgb(192, 230, 239); /* Change to darker gray on hover */
}

/* [?] This is not working? */
.radio-display-box input[type="radio"]:checked {
  background-color: red; 
}

/* Safety and back-up power on or off */
.toggle-state {
    background-color: #f9f9f9;
    padding: 3px;
    border-radius: 5px;
    box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);
    text-align: center;
    color: black;
    background-color: #fac8c8;
}

.chart {
  width: 100%;
  height: 100%;
}

.controls {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
  gap: 10px;
}

.control-box {
  background-color: #f9f9f9;
  padding: 10px; /* Add padding to the entire control box */
  border-radius: 5px;
  box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);
}

.control-box h3{
    margin-bottom: 0.5rem;
    font-size: 1.05rem;
}

.input-btn-flex {
    display: flex;
    justify-content: space-between;
    align-items: center;
    width: 100%;
    margin-bottom: 0.5rem;
}

.input-btn-flex > * {
  flex-grow: 0;
  flex-basis: 50%; /* First element takes up 45% */
  max-width: 50%; /* Optional: Set max-width to ensure responsiveness */
}

.input-btn-flex > :nth-child(2),
.input-btn-flex > :nth-child(3) {
  flex-basis: 20%; /* Second and third elements take up 25% each */
  max-width: 25%; /* Optional: Set max-width to ensure responsiveness */
}

.input-btn-flex h3 {
  margin-top: 10px; /* Add some margin to the right of the heading */
}

input[type="text"] {
    height: 35px; /* Set the height to your desired value */
    text-align: center;/* Add any other styling you want for all text input fields */
    width: 6rem;
    font-size:16px;
}

.send-btn, .download-btn {
  background-color: #3498db;
  font-weight: bold;
  color: #fff;
  border: none;
  padding: 10px 20px;
  border-radius: 5px;
  cursor: pointer;
  text-align: center;
  transition: background-color 0.4s ease;
  
  display: flex; /* Use flexbox */
  justify-content: center; /* Center horizontally */
  align-items: center; /* Center vertically */
}

.send-btn:hover, .download-btn:hover {
  background-color: #2577ae;
}

.download-btn {
  padding: 5px 3.5px;
  background-color:#3498db;
}

.msg {
  color: green;
}

.timestamp {
  display: flex;
  flex-direction: row;
  gap: 2rem;
}

.timestamp p{
  margin-top: 5px;
}

footer {
  background-color: #fff;
  padding: 20px;
  text-align: center;
  box-shadow: 0 -2px 5px rgba(0, 0, 0, 0.1);
  display: flex;
  justify-content: space-around;
  align-items: center;
  flex-wrap: wrap;
}

@media (max-width: 768px) {
  main {
    grid-template-columns: 1fr;
  }

  .controls {
    grid-template-columns: 1fr;
  }
}

</style>

<!-- NORMAL SENSOR MONITOR -->
<body style="background-color: #efefef" onload="process()">

  <!-- MY HTML STARTS HERE -->
  <header>
    <div class="stop-display-box" id="title-estop">
      <h3>E-STOP (18)</h3>
      <button type="button" class="stop-btn" id="btn0" onclick="ButtonPressStop()">STOP</button>
    </div>

    <div id="title">
      <h3>Hopkins Student Wind Energy Team (HWSET) Turbine Control Panel</h3>
    </div>

    <div class="stop-display-box" id="title-estop">
      <h3>EXP (19)</h3>
      <button type="button" class="run-btn" id="btn1" onclick="ButtonPressRun()">RUN</button>
    </div>
  </header>
  <main>
    <section class="data-charts">

      <div class="charts-and-titles">
        <!-- charts on left column -->
        <div class="chart-box">
            <div id="charts" class="chart"></div>
        </div>

        <!-- tags on right column -->
        <div class="chart-text-column">
          <div class="chart-textbox">
            <h3>Voltage (0)</h3>
            <p id="wind-speed-value" class="data-value">0 m/s</p>
          </div>
          
          <div class="chart-textbox">
            <h3>RPM (1)</h3>
            <p id="rpm-value" class="data-value">0</p>
          </div>
        
          <div class="chart-textbox">
            <h3>Current (2)</h3>
            <p id="pwr-value" class="data-value">0 W</p>
          </div>
        </div>
      </div>
   
      <div class="left-display-box">
        <div class="data-display-box">
            <h3>SAFETY STATE (9) </h3>
            <p id="safety" class="toggle-state"> OFF</p>
        </div>

        <div class="data-display-box">
            <h3>BACKUP PWR (10) </h3>
            <p id="backup" class="toggle-state"> OFF</p>
        </div>
      </div>

    </section>

    <!-- Messages: 
        send-op-mode, send-i-load, send-blade-pitch
        radio-button-msg, load-current-msg, blade-pitch-msg
    -->
    <section class="controls">
      <div class="control-box">
        <div class="input-btn-flex">
            <h3>OPERATION MODE </h3>
            <button id="send-op-mode" class="send-btn">CONFIGURE</button>
        </div>
        <div class="radio-group">
            <div class="radio-display-box">
                <input type="radio" id="pwr-optimization" name="mode" checked>
                <label for="pwr-optimization">PWR OPTI (5)</label>
            </div>
         
            <div class="radio-display-box">
                <input type="radio" id="durability" name="mode">
                <label for="durability">DURABILITY (6)</label>
            </div>
                
            <div class="radio-display-box">
                <input type="radio" id="rated-pwr" name="mode">
                <label for="rated-pwr">RATED PWR (7)</label>
            </div>
        </div>
        
        <p id="radio-button-msg" class="msg">CONFIG: Choose a mode, then click on "CONFIGURE".</p>
      </div>
            
      <div class="control-box">
        <div class="input-btn-flex">
          <h3>LOAD CURRENT (0 to 4A): </h3>
          <input type="text" id="new-load-value">
          <!-- <input type="range" class="fanrpmslider" min="0" max="255" value = "0" width = "0%" oninput="UpdateSlider(this.value)"/>
    <br> -->

          <button id="send-i-load" class="send-btn">SEND</button>
        </div>
        <p id="load-current-msg" class="msg">CONFIG: Please enter load current.</p>
      </div>

      <div class="control-box">
        <div class="input-btn-flex">
          <h3>BLADE PITCH (1000 to 2000):</h3>
          <input type="text" id="new-blade-pitch" >
          <button id="send-blade-pitch" class="send-btn">SEND</button>
        </div>
        <p id="blade-pitch-msg" class="msg">CONFIG: Please enter blade pitch state.</p>
      </div>

      <!-- Time stamp and download button -->
      <div class="timestamp">
        <p id="date-time" class="msg">MM-DD-YYYY</p>
        <p id="test-duration", class="msg">Test Duration: 0 mins, 0 sec</p>
      </div>

      <button type="button" class="download-btn" id="downloadTraces" onclick="downloadTracesAsCSV()">DOWNLOAD DATA CSV</button>

      <p class="msg" id="download-msg">Press the button to download experiment results.</p>

    </section>
  </main>
</body>

<script type="text/javascript">
  var time = new Date();
  
  var trace1 = { 
    name: 'Voltage (V)',
    x: [],
    y: [],
    mode: 'lines',
    line: {
      color: '#80CAF6',
      shape: 'spline'
    },
    showlegend: false
  }
  
  var trace2 = {
    name: 'RPM',
    x: [],
    y: [],
    xaxis: 'x2',
    yaxis: 'y2',
    mode: 'lines',
    line: {color: '#DF56F1'},
    showlegend: false
  };
  
  var trace3 = {
    name: 'Current (mA)',
    x: [],
    y: [],
    xaxis: 'x3',
    yaxis: 'y3',
    mode: 'lines',
    line: {color: '#9a9a9a'},
    showlegend: false
  };

  var vw = window.innerWidth * 0.40;
  var vh = 500; // put 3 plots together

  // Define scaling constant for subplot height and spacing
  var plotHeight = 0.22; // Adjust for desired subplot height (0 to 1)
  var plotSpacing = 0.11; 

  var layout = {
    width: vw,
    height: vh,
    margin: {
    t: 0,  // top margin
    b: 40, // bottom margin
    l: 35, // left margin
    r: 10  // right margin
    },

    xaxis: { // WIND SPEED
      type: 'date', 
      tickformat: '%H:%M:%S', 
      tickmode: 'linear', 
      tick0: new Date().getTime(), 
      dtick: 5000, 
      showticklabels: true, 
      domain: [0, 1],
	  showticklabels: false
    },
    yaxis: {
      domain: [plotHeight * 2 + plotSpacing * 2, plotHeight * 3 + plotSpacing * 2], // 0.5, 0.7
      range: [0, 40],
      tickformat: '.2f',
      tickmode: 'array',
      tickvals: [0, 10, 20, 30,40]
    },

    xaxis2: { // RPM
      type: 'date', 
      anchor: 'y2', 
      tickformat: '%H:%M:%S', 
      tickmode: 'linear', 
      tick0: new Date().getTime(), 
      dtick: 5000, 
      showticklabels: true, 
      domain: [0, 1],
	  showticklabels: false

    },
    yaxis2: {
      anchor: 'x2', 
      domain: [plotHeight + plotSpacing, plotHeight * 2 + plotSpacing], //0.25, 0.45 P2
      range: [0, 4000],
      tickmode: 'array',
      tickvals: [0, 1000, 2000, 3000, 4000]
    },

    xaxis3: { // PWR
      type: 'date', 
      anchor: 'y3', 
      tickformat: '%H:%M:%S', 
      tickmode: 'linear', 
      tick0: new Date().getTime(), 
      dtick: 5000, 
      showticklabels: true, 
      domain: [0, 1]
    },
    yaxis3: {
      anchor: 'x3', 
      domain: [0, plotHeight],
      range: [0, 4000],
      tickmode: 'array',
      tickvals: [0, 1000, 2000, 3000, 4000]
    }
  }

  var data = [trace1,trace2,trace3]; 
  Plotly.newPlot('charts', data, layout, { displayModeBar: false });
  
  var cnt = 0;
  
  // global variable visible to all java functions
  var xmlHttp = createXmlHttpObject();

  // function to create XML object
  function createXmlHttpObject() {
    if (window.XMLHttpRequest) {
      xmlHttp = new XMLHttpRequest();
    } else {
      xmlHttp = new ActiveXObject("Microsoft.XMLHTTP");
    }
    return xmlHttp;
  }

  /* Buttons that send info back to the ESP */
  function ButtonPressStop() {
    var xhttp = new XMLHttpRequest();
    xhttp.open("PUT", "BUTTON_STOP", false);
    xhttp.send();
  }

  function ButtonPressRun() {
    var xhttp = new XMLHttpRequest();
    xhttp.open("PUT", "BUTTON_RUN", false);
    xhttp.send();
    startDataCollection();
  }

  function SetOperationMode(mode_str) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        // update the web based on reply from ESP
        radioMsg.textContent = `SUCCESS: Mode set to "${mode_str}".`; 
      }
    }
  
    if (mode_str == "PWR OPTI (5)") {
      xhttp.open("PUT", "PWR_OPTI", true); 
      xhttp.send();
    } else if (mode_str == "DURABILITY (6)") {
        xhttp.open("PUT", "DURABILITY", true); 
        xhttp.send();
    } else if (mode_str == "RATED PWR (7)") {
        xhttp.open("PUT", "RATED_PWR", true); 
        xhttp.send();
    } else {
        radioMsg.textContent = 'ERROR: Invalid mode selected.'; 
        return;
    }
  }

  function UpdateLoadCurrent(value) {
    var xhttp = new XMLHttpRequest();
    xhttp.open("PUT", "UPDATE_LOAD_CURRENT?VALUE="+value, true);
    xhttp.send();
  }
  function UpdatePitch(value) {
    var xhttp = new XMLHttpRequest();
    xhttp.open("PUT", "UPDATE_PITCH?VALUE="+value, true);
    xhttp.send();
  }

  // function to handle the response from the ESP
  function response() {

    // I want "plot-stream" to update here, so that it can plot a random number every second (when var dt updates)
    var msg_B1;
    var msg_V0;
    var msg_V1;

    var msg; // stop light

    var barwidth;
    var currentsensor;
    var xmlResponse;
    var xmldoc;
    var dt = new Date();
    var color = "#e8e8e8";

    // get the xml stream
    xmlResponse = xmlHttp.responseXML;

    // get host date and time
    formattedTime = dt.toLocaleTimeString();
    formattedDate = dt.toLocaleDateString();
    document.getElementById("date-time").textContent = "Time: " + formattedDate + " " + formattedTime;
    
    // wind speed
    xmldoc = xmlResponse.getElementsByTagName("wind-speed");  
    msg_V0 = xmldoc[0].firstChild.nodeValue;

    if (msg_V0 > 0.5) {
      color = "#aa0000";
    } else {
      color = "#0000aa";
    }

    document.getElementById("wind-speed-value").textContent = msg_V0 + " V";
    document.getElementById("wind-speed-value").style.color = color;
    
    // rpm
    xmldoc = xmlResponse.getElementsByTagName("rpm");
    msg_B1 = xmldoc[0].firstChild.nodeValue;

    document.getElementById("rpm-value").textContent = msg_B1;
    document.getElementById("rpm-value").style.color = color;

    // pwr
    xmldoc = xmlResponse.getElementsByTagName("pwr");
    msg_V1 = xmldoc[0].firstChild.nodeValue;
    document.getElementById("pwr-value").textContent = msg_V1 + " mA";
    document.getElementById("pwr-value").style.color = color;

    // [TODO] e-load measured currnet
    xmldoc = xmlResponse.getElementsByTagName("load-current-measured");
    msg_load_current_measured = xmldoc[0].firstChild.nodeValue;

    // [TODO] e-load measured currnet
    //xmldoc = xmlResponse.getElementsByTagName("pitch-measured");
    //msg_pitch_measured = xmldoc[0].firstChild.nodeValue;
    document.getElementById("blade-pitch-msg").textContent = "Current pitch: " + msg_load_current_measured;

    // Toggle stop button on pin 13
    xmldoc = xmlResponse.getElementsByTagName("STOP");
    msg = xmldoc[0].firstChild.nodeValue;
    if (msg == 0) {
      document.getElementById("btn0").textContent = "STOP";
      document.getElementById("btn0").style.backgroundColor = "#a2a2a2";
    } else {
      document.getElementById("btn0").textContent = "STOPPED";
      document.getElementById("btn0").style.backgroundColor = "#e74c3c";
    }

    // Toggle exp run button on pin 11
    xmldoc = xmlResponse.getElementsByTagName("RUN");
    msg = xmldoc[0].firstChild.nodeValue;
    if (msg == 0) { // not running
      document.getElementById("btn1").textContent = "PAUSED";
      document.getElementById("btn1").style.backgroundColor = "#a2a2a2";
    } else { // running
      document.getElementById("btn1").textContent = "RUNNING";
      document.getElementById("btn1").style.backgroundColor = "#3df5aa";
    }

    // Toggle safety state box on pin 45
    xmldoc = xmlResponse.getElementsByTagName("SAFETY");
    msg = xmldoc[0].firstChild.nodeValue;
    if (msg == 0) { // not running
      document.getElementById("safety").textContent = "OFF";
      document.getElementById("safety").style.backgroundColor = "#f2f2f2";
    } else { // running
      document.getElementById("safety").textContent = "ON";
      document.getElementById("safety").style.backgroundColor = "#e74c3c";
    }

    // Toggle backup power state box on pin 46
    xmldoc = xmlResponse.getElementsByTagName("BACKUP");
    msg = xmldoc[0].firstChild.nodeValue;
    if (msg == 0) { // not running
      document.getElementById("backup").textContent = "OFF";
      document.getElementById("backup").style.backgroundColor = "#f2f2f2";
    } else { // running
      document.getElementById("backup").textContent = "ON";
      document.getElementById("backup").style.backgroundColor = "#e74c3c";
    }

    // Plots: Update
    var x_axis_interval = 50;

    var update = {
      x: [[dt], [dt], [dt]],
      y: [[msg_V0], [msg_B1], [msg_V1]]
    }
    
    Plotly.extendTraces('charts', update, [0,1,2], x_axis_interval)
   
  }

  function process() {
    if (xmlHttp.readyState == 0 || xmlHttp.readyState == 4) {
      xmlHttp.open("PUT", "xml", true);
      xmlHttp.onreadystatechange = response;
      xmlHttp.send(null);
    }
    setTimeout("process()", 400);
  }

// -------------------- DOWNLOAD REPORT FORMATTING ---------------

// ----------------- DOWNLOAD BUTTON -------------------
// global variable to hold URL to file to be downloaded (see generateCSVUrl())
 let csvFileUrl = null;

// Function for generating a text file URL containing given text
function generateCSVUrl(txt) {
    let fileData = new Blob([txt], {type: 'text/csv'});
    // If a file has been previously generated, revoke the existing URL
    if (csvFileUrl !== null) {
        window.URL.revokeObjectURL(csvFileUrl);
    }
    csvFileUrl = window.URL.createObjectURL(fileData);
    // Returns a reference to the global variable holding the URL
    // Again, this is better than generating and returning the URL itself from the function as it will eat memory if the file contents are large or regularly changing
    return csvFileUrl;
};

// Function for downloading file from URI
function downloadURI(uri, name) {
  let link = document.createElement("a");
  link.download = name;
  link.href = uri;
  document.body.appendChild(link);
  link.click();
  document.body.removeChild(link);
}

let csvData = ""; // Global variable to hold CSV data

// Function to start data collection (call this when "Start Experiment Trial" button is clicked)
function startDataCollection() {
    for (let i = 0; i < trace1.x.length; i++) {
      let x1 = trace1.x[i].toLocaleTimeString('en-US')
      csvData += `\n${x1}, ${trace1.y[i]}, ${trace2.y[i]}, ${trace3.y[i]}`;
    }
}

// Function to generate CSV data and download CSV file (call this when "Download CSV" button is clicked)
function downloadTracesAsCSV() {
  csvData += `\n\n\n`;
  startDataCollection(); // Start data collection
  let today = new Date();
  let header = `HSWET Turbine Dashboard Download,\n${today}\nTime, Trace1, Trace2, Trace3\n`;
  let finalCSVData = header + csvData; // Combine header and collected data

  // Generate CSV URL
  let url = generateCSVUrl(finalCSVData);

  const dt = new Date();  
  const year = dt.getFullYear();
  const month = String(dt.getMonth() + 1).padStart(2, "0");
  const day = String(dt.getDate()).padStart(2, "0");
  const hours = String(dt.getHours()).padStart(2, "0");
  const minutes = String(dt.getMinutes()).padStart(2, "0");

  const formattedDateTime = `${month}_${day}_${year}_${hours}_${minutes}`;
  downloadURI(url, formattedDateTime + "_turbine_data.csv");

  document.getElementById("download-msg").textContent = "SUCCESS: Downloaded data CSV at " + formattedDateTime;
  csvData = "";
}

// Button Interactions

const radioMsg = document.getElementById('radio-button-msg'); // Define radioMsg globally

document.addEventListener('DOMContentLoaded', function() {
  // Get elements
  const radioButtons = document.querySelectorAll('input[name="mode"]');
  const sendButton = document.getElementById('send-op-mode');
 // const radioMsg = document.getElementById('radio-button-msg'); // Updated variable

 // Add event listeners to radio buttons
 radioButtons.forEach(radio => {
      radio.addEventListener('change', function() {
          const selectedRadio = document.querySelector('input[name="mode"]:checked');
          // Reset color of any previously selected label
          
          if (selectedRadio) {
              const selectedLabel = selectedRadio.nextElementSibling.textContent;
              // Do not update console message here
          }
      });
  });

  // Add event listener to send button
  sendButton.addEventListener('click', function() {
      const selectedRadio = document.querySelector('input[name="mode"]:checked');

      if (selectedRadio) {
          const selectedLabel = selectedRadio.nextElementSibling.textContent;
          // Send the XML messaaage to the ESP
          SetOperationMode(selectedLabel);

          // [NEW] Add to CSV
          let x1 = trace1.x[i].toLocaleTimeString('en-US')
          csvData += `\n${x1},,, ${selectedLabel}`;
        } else {
          radioMsg.textContent = 'ERROR: Please select a mode.';
      }
  });
});

document.addEventListener('DOMContentLoaded', function() {
  // Get elements
  const newLoadValueInput = document.getElementById('new-load-value');
  const sendButton = document.getElementById('send-i-load');
  const loadCurrentMsg = document.getElementById('load-current-msg');

  let previousValidValue = 0; // Variable to store the previous valid value
  let measuredValue = 0; 

  sendButton.addEventListener('click', function() {
      loadCurrentMsg.textContent = 'Updated.';
      // Get the new value from the input
      const inputValue = newLoadValueInput.value.trim(); // Remove leading and trailing whitespace
      const decimalCount = (inputValue.match(/\./g) || []).length; // Count the number of decimal points

      if (decimalCount <= 1) {
          // Parse the input value as a float with one decimal point
          const newValue = parseFloat(inputValue).toFixed(3);

          // This clause actually updates and verifies the load current value
          if (!isNaN(newValue) && newValue >= 0 && newValue <= 4) {
              // [PLACEHOLDER]
              // const randomFactor = Math.random() * (1.002 - 0.998) + 0.998; // Random number between 0.998 and 1.002
              // const measuredValue = newValue * randomFactor;
              //measuredValue = msg_load_current_measured; // [TODO] This should get the actually measured current

              // [TODO] NEED TO ACTUALLY TALK TO ADC (one pin for send, one pin for receive)
              UpdateLoadCurrent(newValue*1000);

              // [NEW] Add to CSV
              let x1 = trace1.x[trace1.x.length-1].toLocaleTimeString('en-US')
              csvData += `\n${x1},,, ${newValue}`;
 
              // Update the displayed value and success message
              loadCurrentMsg.textContent = `SUCCESS: Set to ${newValue} A.`;

              

              // Save the new value as the previous valid value
              previousValidValue = newValue;

              // Clear the input field
              newLoadValueInput.value = newValue;
          } else {
              // Display error message for out-of-range input
              loadCurrentMsg.textContent = `ERROR: Input out of bounds.`;

              // Populate the input field with the previous valid value
              newLoadValueInput.value = previousValidValue;
          }
      } else {
          // Display error message for multiple decimal points
          loadCurrentMsg.textContent = 'ERROR: Input must contain only one decimal point.';
      }
  });
});

document.addEventListener('DOMContentLoaded', function() {
  // Get elements
  const newBladePitchInput = document.getElementById('new-blade-pitch');
  const sendBladePitchButton = document.getElementById('send-blade-pitch');
  const bladePitchMsg = document.getElementById('blade-pitch-msg');

  let previousValidPitch = 0; // Variable to store the previous valid pitch

  // Update value when SEND button is clicked
  sendBladePitchButton.addEventListener('click', function() {
    // Get the new pitch value from the input
    const newPitchValue = parseInt(newBladePitchInput.value.trim());

    // Check if the input value is a valid integer between 0 and 5
    if (!isNaN(newPitchValue) && Number.isInteger(newPitchValue) && newPitchValue >= 1000 && newPitchValue <= 2000) {
      UpdatePitch(newPitchValue);

      // [NEW] Add to CSV
      let x1 = trace1.x[trace1.x.length-1].toLocaleTimeString('en-US')
      csvData += `\n${x1},, ${newPitchValue},`;

      // Update the success message
      bladePitchMsg.textContent = `SUCCESS: Blace pitch state is set to ${newPitchValue}.`;

      // Save the new pitch value as the previous valid value
      previousValidPitch = newPitchValue;

      // Clear the input field
      newBladePitchInput.value = newPitchValue;
    } else {
      // Display error message for invalid input
      bladePitchMsg.textContent = `ERROR: Input out of bounds. Pitch state is ${previousValidPitch}.`;

      // Populate the input field with the previous valid value
      newBladePitchInput.value = previousValidPitch;
    }
  });
});

</script>

</html> )=====";