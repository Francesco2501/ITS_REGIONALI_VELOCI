using System;

namespace ITS.PW_ANNO2.GRUPPO4.Dashboard.Models
{
    public class Item
    {
        public int ntrain { get; set; }
        public int ncarriage { get; set; }
        public int? temp { get; set; }
        public int? humidity { get; set; }
        public Boolean? bathdoor { get; set; }
        public Boolean? alarm { get; set; }
        public Boolean? carriagedoor1 { get; set; }
        public Boolean? carriagedoor2 { get; set; }
        //public DateTime savetime { get; set; }
    }
}
