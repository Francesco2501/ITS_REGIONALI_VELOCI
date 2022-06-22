using ITS.PW_ANNO2.GRUPPO4.Dashboard.Models;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace ITS.PW_ANNO2.GRUPPO4.Dashboard.Services
{
    public interface ICosmosDbService
    {
        Task<IEnumerable<Item>> GetMultipleAsync(string query);
        Task<Item> GetAsync(string id);
        Task AddAsync(Item item);
        Task UpdateAsync(string id, Item item);
        Task DeleteAsync(string id);
    }
}
