using Microsoft.Azure.Devices;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace ITS.PW_ANNO2.GRUPPO4.DeviceToCloud
{
    public class Worker : BackgroundService
    {
        private readonly ILogger<Worker> _logger;
        private readonly ServiceClient _serviceClient;

        public Worker(ILogger<Worker> logger, IConfiguration configuration)
        {
            _logger = logger;
            var cs = configuration.GetConnectionString("ServiceCs");
            _serviceClient = ServiceClient.CreateFromConnectionString(cs);
        }

        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            while (!stoppingToken.IsCancellationRequested)
            {
                /*var p = new Product();
                var json = JsonSerializer.Serialize(p);
                var o = JsonSerializer.Deserialize(json);*/
                Console.WriteLine("Device di destinazione");
                string deviceId = Console.ReadLine();
                Console.WriteLine("testo da inviare");
                string text = Console.ReadLine();

                var message = new Message(Encoding.ASCII.GetBytes(text));
                await _serviceClient.SendAsync(deviceId, message);

                await Task.Delay(1000, stoppingToken);

            }
        }

    }
}
