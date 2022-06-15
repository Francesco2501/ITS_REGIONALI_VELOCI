using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using uPLibrary.Networking.M2Mqtt;
using uPLibrary.Networking.M2Mqtt.Messages;

namespace Gateway_ProjectWork
{
    class MqttHelper
    {
        MqttClient client;
        string clientId = "Gateway-RV3569";

        public MqttHelper()
        {
            string BrokerAddress = "broker.mqttdashboard.com";

            client = new MqttClient(BrokerAddress);

            string topic = $"pw4/command/{clientId}/#";

            // subscribe to the topic with QoS 2
            client.Subscribe(new string[] { topic }, new byte[] { 2 });   // we need arrays as parameters because we can subscribe to different topics with one call

            client.MqttMsgPublishReceived += client_MqttMsgPublishReceived;

            client.Connect(clientId);
        }

        // this code runs when a message was received
        void client_MqttMsgPublishReceived(object sender, MqttMsgPublishEventArgs e)
        {
            string ReceivedMessage = Encoding.UTF8.GetString(e.Message);
            Console.WriteLine($"Comando ricevuto: => {ReceivedMessage}");
        }

        public void SendMessage(string wagon, string message)
        {
            // whole topic
            string topic = $"pw4/status/{clientId}/{wagon}";
            
            // publish a message with QoS 2
            client.Publish(topic, Encoding.UTF8.GetBytes(message), MqttMsgBase.QOS_LEVEL_EXACTLY_ONCE, true);
            Console.WriteLine($"Status inviato al broker mqtt");
        }

    }
}
