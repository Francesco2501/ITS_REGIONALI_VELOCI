using Microsoft.AspNetCore.Mvc;
using Microsoft.AspNetCore.Mvc.RazorPages;
using Microsoft.Azure.Devices;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.Logging;
using System;
using System.Text;
using System.Text.Json;

namespace ITS.PW_ANNO2.GRUPPO4.Dashboard.Pages.Control
{
    public class SendMessageModel : PageModel
    {
        private readonly ILogger<SendMessageModel> _logger;
        private readonly ServiceClient _serviceClient;

        public SendMessageModel(ILogger<SendMessageModel> logger, IConfiguration configuration)
        {
            _logger = logger;
            var cs = configuration.GetConnectionString("ServiceCs");
            _serviceClient = ServiceClient.CreateFromConnectionString(cs);
        }

        public void OnPostTemp()
        {
            string deviceId = "raspberrydevice";
            var nt = Convert.ToInt16(Request.Form["nttrain"]);
            var nc = Convert.ToInt16(Request.Form["ntcarriage"]);
            var tempc = 0;
            if (nc != 0) { tempc = nc; }

            var settemperature = Convert.ToDouble(Request.Form["temp-value"]);
            
            var obj = new { ntrain = nt,  ncarriage = tempc, settemperature = settemperature};
            string jsonString = JsonSerializer.Serialize(obj);           

            var commandMessage = new Message(Encoding.UTF8.GetBytes(jsonString));
            _serviceClient.SendAsync(deviceId, commandMessage);
        }

        public void OnPostAlarm()
        {
            string deviceId = "raspberrydevice";
            var nt = Convert.ToInt16(Request.Form["ntrain"]);
            var nc = Convert.ToInt16(Request.Form["ncarriage"]);
            var alarmc = 0;
            if (nc != 0) { alarmc = nc; }

            var al = Convert.ToInt16(Request.Form["alarm"]);
            var alarm = false;
            if (al == 1) { alarm = true; }
            

            var obj = new { ntrain = nt, ncarriage = alarmc, alarm = alarm };
            string jsonString = JsonSerializer.Serialize(obj);

            var commandMessage = new Message(Encoding.UTF8.GetBytes(jsonString));
            _serviceClient.SendAsync(deviceId, commandMessage);
        }
    }
}
