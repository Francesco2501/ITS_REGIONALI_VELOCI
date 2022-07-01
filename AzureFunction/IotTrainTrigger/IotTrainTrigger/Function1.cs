using IoTHubTrigger = Microsoft.Azure.WebJobs.EventHubTriggerAttribute;

using Microsoft.Azure.WebJobs;
using Microsoft.Azure.WebJobs.Host;
using Microsoft.Azure.EventHubs;
using System.Text;
using System.Net.Http;
using Microsoft.Extensions.Logging;
using System;
using System.Text.Json;

namespace IotTrainTrigger
{
    public class DataGruppo4
    {
        public long ts { get; set; }
        public int ntrain { get; set; }
        public int ncarriage { get; set; }

        public Double? settemperature { get; set; }
        public Double? temp { get; set; }
        public Double? humidity { get; set; }
        public Boolean? bathdoor { get; set; }
        public Boolean? alarm { get; set; }
        public Boolean? carriagedoor1 { get; set; }
        public Boolean? carriagedoor2 { get; set; }
        
    }
    public static class Function1
    {


        //private static HttpClient client = new HttpClient();

        [FunctionName("Function1")]
        public static void Run([IoTHubTrigger("messages/events", Connection = "HubConnectionString")] EventData message,
            [CosmosDB(
                databaseName: "datatrain",
                collectionName: "raspberry-iot",
                ConnectionStringSetting = "DbConnectionString")]out dynamic document, ILogger log)
        {
            var data = JsonSerializer.Deserialize<DataGruppo4>(Encoding.UTF8.GetString(message.Body));
            object dataTrain = data;

            //if (data.temp != null && data.humidity != null && data.bathdoor != null && data.alarm != null && data.carriagedoor1 != null && data.carriagedoor2 != null)
            dataTrain = new { data.ts, data.settemperature, data.ntrain, data.ncarriage, data.temp, data.humidity, data.bathdoor, data.alarm, data.carriagedoor1, data.carriagedoor2 };
            //if (data.bathdoor != null)
            //    dataTrain = new { data.ntrain, data.ncarriage, data.bathdoor };
            //else if (data.alarm != null)
            //    dataTrain = new { data.ntrain, data.ncarriage, data.alarm };
            //else if (data.carriagedoor1 != null)
            //    dataTrain = new { data.ntrain, data.ncarriage, data.carriagedoor1 };
            //else if (data.carriagedoor2 != null)
            //    dataTrain = new { data.ntrain, data.ncarriage, data.carriagedoor2 };

            document = dataTrain;

            log.LogInformation($"C# IoT Hub trigger function processed a message: {Encoding.UTF8.GetString(message.Body.Array)}");
        }
    }
}