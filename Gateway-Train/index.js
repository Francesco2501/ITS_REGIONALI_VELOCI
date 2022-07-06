// --- SERIAL PORT --- ///

const SerialPort = require('serialport');
//const Readline = require('@serialport/parser-readline')
const ByteLength = require('@serialport/parser-byte-length');

const port = new SerialPort('/dev/ttyUSB0', {
  baudRate: 9600
});

const parser = port.pipe(new ByteLength({ length: 1 }))
const train = 1;

//byte-type conversion
const typesDictionary = {
  0x01 : "TempHum",
  0x04 : "DesiredTemperature",
  0x08 : "Toilet",
  0x10 : "CoachDoor",
  0x20 : "Alarm",
  0x40 : "Address"
};

//TO BE POPULTATED USING ADDRESS REQUESTS
//array of connected coaches
const coachAddresses = [0x01];

//dictionary containing stauses of connected coaches
var coaches = {};

var dataPacket = [];



// --- CLOUD --- ///
'use strict';


var DeviceClient = require('azure-iot-device').Client;  // npm i azure-iot-device
var Message = require('azure-iot-device').Message;
var Mqtt = require('azure-iot-device-mqtt').Mqtt;   // npm i azure-iot-device-mqtt

var connectionString = "HostName=Iot-Train-Hub.azure-devices.net;DeviceId=raspberrydevice;SharedAccessKey=c5KBx8uj6aLgfg3hsyG5BIZYkudRPdVWbphhjTJgvno="
var targetDevice = 'raspberrydevice';


var client = DeviceClient.fromConnectionString(connectionString, Mqtt);


function connectHandler () {
}


client.on('connect', connectHandler);

client.open()
.catch(err => {
  console.error('Could not connect: ' + err.message);
});




// --- REDIS QUEUE --- //

const redis = require('redis')

const redisClient = redis.createClient({
    host: '127.0.0.1',
    port: 6379
});



async function OpenRedisConnection() {
  await redisClient.connect()
}

OpenRedisConnection();







// SERIAL PORT --- //

//REQUEST TEMP HUM SAMPLE
function RequestTempHum(address){
  var body = [0xFF, address, 0x01, 0xFE]

  port.write(body, function(err) {
      if (err) {
        return console.log('Error on write: ', err.message)
      }
      console.log('message written')
})}

function RequestAddress(){
  var body = [0xFF, 0x23, 0x40, 0xFE]

  port.write(body, function(err) {
      if (err) {
        return console.log('Error on write: ', err.message)
      }
      console.log('message written')
})}

function SendDesiredTemp(address, value){
  let intPart = Math.floor(value);
  decimalPart = Math.round((value - Math.floor(value))*10);
  var body = [0xFF, address, 0x04, intPart, decimalPart, 0xFE]

  port.write(body, function(err) {
      if (err) {
        return console.log('Error on write: ', err.message)
      }
      console.log('message written')
})}

function SendAlarm(address, value){

  var body = [0xFF, address, 0x20, value, 0xFE]
  port.write(body, function(err) {
      if (err) {
        return console.log('Error on write: ', err.message)
      }
      console.log('message written')
})}

function UpdateTempHums(){
  for (const [key, value] of Object.entries(coaches)) {
    setTimeout(RequestTempHum, 1000, key);
}
}
//#########################
// Open errors will be emitted as an error event
port.on('error', function(err) {
console.log('Error: ', err.message)
})

parser.on('data', composePacket)

var partCounter = 0;
//function that recomposes received data packet
function composePacket(data){
  if(data[0] == 0xFF){
    dataPacket = [];
    partCounter = 0;
  }
  if(data[0] == 0xFE){
    handlePacket(dataPacket)
  }
  dataPacket[partCounter] = data[0]
  partCounter ++;
}

//handle the received data packet and update statuses
var type = "";
var address = 0;
function handlePacket(packet){
  address = packet[1];
  type = typesDictionary[packet[2]];
  if(type == "TempHum"){
    coaches[address].temp = manageTemperature(packet[3], packet[4]);
    coaches[address].humidity = packet[5] + packet[6]*0.1;
  }
  if(type == "CoachDoor"){
    coaches[address].carriagedoor1 = packet[3];
  }
  if(type == "Toilet"){
    coaches[address].bathdoor = packet[3];
  }
  if(type == "Alarm"){
    coaches[address].alarm = packet[3];
  }
  if(type == "DesiredTemperature"){
    coaches[address].settemperature = packet[3] + packet[4]*0.1;
  }
  if(type == "Address"){
    if(!(address in coaches)){
      coaches[address] = {
        ts: 0,
        ntrain: train,
        ncarriage: address,
        carriagedoor1: 0,
        carriagedoor2: 0,
        bathdoor: 0,
        alarm:0,
        temp: 0.0,
        humidity: 100.0,
        settemperature: 0.0
      };
    }
  }
  console.log(coaches[address]);

  SendData(coaches[address]);

}

//convert received bytes to temperature
function manageTemperature(intPart, decimalPart){
  let temperature = 0;
  if(intPart>127){

    temperature = ((intPart-128)+(0.1*decimalPart))*(-1);
  }
  else{
    temperature += intPart;
    temperature += (0.1*decimalPart);
  }
  return temperature;
}

RequestAddress();
setInterval(RequestAddress,10000);
setInterval(UpdateTempHums,5000);






////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// --- IOT-DEVICE --- //


// Received data

client.on('message', function (msg) {
    var jsonData = JSON.parse(msg.data);

    if (jsonData.hasOwnProperty('settemperature')) {
        console.log("Settemp")
        SendDesiredTemp(1, jsonData.settemperature)
    }

    if (jsonData.hasOwnProperty('alarm')) {
        console.log("Alarm")
        if (jsonData.alarm == true) {
            jsonData.alarm = 1;
        }
        else {
            jsonData.alarm = 0;
        }
        SendAlarm(1, jsonData.alarm);
    }

    console.log(jsonData)

});







// // Funziona send

// var jsonData = {
//   ts : Date.now(),
//   ntrain : 1,
//   ncarriage : 5,
//   temp : 20.5,
//   humidity : 62,
//   bathdoor : true,
//   alarm : false,
//   carriagedoor1 : true,
//   carriagedoor2 : false,
//   settemperature : 19.5               
// };

// var jsonData = {
//   ts : Date.now(),
//   ntrain : 1,
//   ncarriage : 5,
//   alarm : false,
// };

// var jsonData = {
//   ts : Date.now(),
//   ntrain : 1,
//   ncarriage : 5,
//   settemperature : 19.5               
// };

 






function SendData(jsonData){

    jsonData = ManageJson(jsonData);
    CheckConnectivity(jsonData);
    GetDataFromQueue();

    var message = new Message(JSON.stringify(jsonData));
    console.log('Sending message: ' + message.getData());
    client.sendEvent(message, printResultFor('send'));
  }

  function SendDataToCloud(jsonData){

    var message = new Message(JSON.stringify(jsonData));
    console.log('Sending message: ' + message.getData());
    client.sendEvent(message, printResultFor('send'));

  }

  function InsertDataToQueue(jsonData){
    var dataStr = JSON.stringify(jsonData)
    redisClient.lPush('TrainQueue', dataStr)
  }

  function GetDataFromQueue(){

    redisClient.rPop('TrainQueue')
    .then((data) => {
      if (data != null) {
        CheckConnectivity(data)
        GetDataFromQueue();
      }
      else {
        console.log("La coda è vuota")
      }
    })
    .catch((err)=> console.log(err))

  }

  function CheckConnectivity(jsonData) {

    require('dns').resolve('www.google.com', function(err) {
      if (err) {
         console.log("No connection");
         InsertDataToQueue(jsonData);

      } else {
         console.log("Connected");
         SendDataToCloud(jsonData);

      }
    });

  }

  function ManageJson(jsonData) {
    if (jsonData.hasOwnProperty('ts')) {
        jsonData.ts = Date.now();
    }

    if (jsonData.hasOwnProperty('ntrain')) {
        jsonData.ntrain = Math.floor(Math.random() * 3) + 1;
    }

    if (jsonData.hasOwnProperty('ncarriage')) {
        jsonData.ncarriage = Math.floor(Math.random() * 5) + 1;
    }

    if (jsonData.hasOwnProperty('carriagedoor1')) {
        if (jsonData.carriagedoor1 == 1) {
            jsonData.carriagedoor1 = true;
        }
        else {
            jsonData.carriagedoor1 = false;
        }
    }

    if (jsonData.hasOwnProperty('carriagedoor2')) {
        if (jsonData.carriagedoor2 == 1) {
            jsonData.carriagedoor2 = true;
        }
        else {
            jsonData.carriagedoor2 = false;
        }
    }

    if (jsonData.hasOwnProperty('bathdoor')) {
        if (jsonData.bathdoor == 1) {
            jsonData.bathdoor = true;
        }
        else {
            jsonData.bathdoor = false;
        }
    }

    if (jsonData.hasOwnProperty('alarm')) {
        if (jsonData.alarm == 1) {
            jsonData.alarm = true;
        }
        else {
            jsonData.alarm = false;
        }
    }

    return jsonData;
  }

  // Helper function to print results in the console
  function printResultFor(op) {
    return function printResult(err, res) {
      if (err) console.log(op + ' error: ' + err.toString());
      if (res) console.log(op + ' status: ' + res.constructor.name);
    };
  }

