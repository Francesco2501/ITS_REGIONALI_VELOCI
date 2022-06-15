using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;
using StackExchange.Redis;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Net;
using System.Threading;
using System.Threading.Tasks;

namespace Gateway_ProjectWork
{
    public class Worker : BackgroundService
    {
        private readonly ILogger<Worker> _logger;
        private readonly MqttHelper _mqtt;

        public Worker(ILogger<Worker> logger)
        {
            _logger = logger;
            _mqtt = new MqttHelper();
        }

        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            while (!stoppingToken.IsCancellationRequested)
            {
                var r = new Random();
                string data = $"Vagone-{r.Next(1,5)}: Temperatura: {r.Next(16, 28)}";

                if (data != null)
                {
                    CheckQueue();

                    string key = Guid.NewGuid().ToString();
                    UploadData(key, data);

                }

                System.Threading.Thread.Sleep(2000);
            }
        }

        public void CheckQueue()
        {
            Console.WriteLine($"Controllo se sono presenti elementi nella coda...");
            System.Threading.Thread.Sleep(500);

            var cache = RedisConnectorHelper.Connection.GetDatabase();
            var keys = RedisConnectorHelper.GetAllKeys();
            var value = "";

            if (keys.Count > 0)
                Console.WriteLine($"Coda ha dei dati");
            else
                Console.WriteLine($"Coda vuota");
            System.Threading.Thread.Sleep(500);



            keys.ForEach(key =>
            {
                value = cache.StringGet(key);
                ReuploadData(key, value);
            });
        }

        public void UploadData(string key, string value)
        {
            try
            {
                if (CheckForInternetConnection())
                {
                    _mqtt.SendMessage(value[7].ToString(), value);

                    Console.WriteLine($"Dato Salvato in Cloud [value={value}]");
                }
                else
                    SaveToQueue(key, value);
                System.Threading.Thread.Sleep(200);

            }
            catch (Exception)
            {
                SaveToQueue(key, value);
            }
        }

        public void ReuploadData(string key, string value)
        {
            try
            {
                if (CheckForInternetConnection())
                {
                    _mqtt.SendMessage(value[7].ToString(), value);

                    RedisConnectorHelper.RemoveKey(key);
                    Console.WriteLine($"Dato Ricaricato in Cloud dalla coda [value={value}]");
                }
                System.Threading.Thread.Sleep(200);

            }
            catch (Exception)
            {
            }
        }

        public void SaveToQueue(string key, string value)
        {
            var cache = RedisConnectorHelper.Connection.GetDatabase();
            cache.StringSet(key, value);
            Console.WriteLine($"ERRORE => dato salvato in redis [value={value}]");
        }

        public bool CheckForInternetConnection(int timeoutMs = 10000, string url = "https://www.google.it/")
        {
            try
            {
                url ??= CultureInfo.InstalledUICulture switch
                {
                    { Name: var n } when n.StartsWith("fa") => // Iran
                        "http://www.aparat.com",
                    { Name: var n } when n.StartsWith("zh") => // China
                        "http://www.baidu.com",
                    _ =>
                        "http://www.gstatic.com/generate_204",
                };

                var request = (HttpWebRequest)WebRequest.Create(url);
                request.KeepAlive = false;
                request.Timeout = timeoutMs;
                using (var response = (HttpWebResponse)request.GetResponse())
                    return true;
            }
            catch
            {
                return false;
            }
        }
    }
}
