import { initializeApp } from "https://www.gstatic.com/firebasejs/10.7.1/firebase-app.js";
import {
  getDatabase,
  ref,
  onValue,
  update,
} from "https://www.gstatic.com/firebasejs/10.7.1/firebase-database.js";

const firebaseConfig = {
  apiKey: "",
  authDomain: "",
  databaseURL: "",
  projectId: "",
  storageBucket: "",
  messagingSenderId: "",
  appId: "",
};

const app = initializeApp(firebaseConfig);
const db = getDatabase();

var sensorReading = document.getElementById("sensor-reading");

const sensorDataRef = ref(db, "Sensor/sensor_data");
onValue(sensorDataRef, (snapshot) => {
  const data = snapshot.val();
  sensorReading.innerText = data;
});

var slider = document.getElementById("myRange");
var output = document.getElementById("output");
output.innerHTML = slider.value;

slider.oninput = function () {
  output.innerHTML = this.value;
};

function manualTing() {
  var value = document.getElementById("manual-label").checked;
  document.getElementById("myRange").disabled = !value;
  update(ref(db), {
    manual: value,
  });
}
document.getElementById("manual-label").addEventListener("click", manualTing);

function manualSlider() {
  var value = parseInt(document.getElementById("myRange").value);
  update(ref(db, "Sensor"), {
    duty_cycle: value,
  });
}
document.getElementById("myRange").addEventListener("input", manualSlider);
