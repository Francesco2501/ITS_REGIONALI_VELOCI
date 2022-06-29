using Newtonsoft.Json;
using System;

namespace ITS.PW_ANNO2.GRUPPO4.Dashboard.Models
{
    public class Item
    {
        [JsonProperty(PropertyName = "id")]
        public string Id { get; set; }
        [JsonProperty(PropertyName = "ntrain")]
        public int Ntrain { get; set; }
        [JsonProperty(PropertyName = "ncarriage")]
        public int Ncarriage { get; set; }
        [JsonProperty(PropertyName = "temp")]
        public int? Temp { get; set; }
        [JsonProperty(PropertyName = "humidity")]
        public int? Humidity { get; set; }
        [JsonProperty(PropertyName = "bathdoor")]
        public Boolean? Bathdoor { get; set; }
        [JsonProperty(PropertyName = "alarm")]
        public Boolean? Alarm { get; set; }
        [JsonProperty(PropertyName = "carriagedoor1")]
        public Boolean? Carriagedoor1 { get; set; }
        [JsonProperty(PropertyName = "carriagedoor2")]
        public Boolean? Carriagedoor2 { get; set; }
        [JsonProperty(PropertyName = "ts")]
        public DateTime Savetime { get; set; }
    }
}
