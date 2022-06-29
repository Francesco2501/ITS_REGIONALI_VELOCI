using ITS.PW_ANNO2.GRUPPO4.Dashboard.Models;
using ITS.PW_ANNO2.GRUPPO4.Dashboard.Services;
using Microsoft.AspNetCore.Mvc;
using System;
using System.Threading.Tasks;

namespace ITS.PW_ANNO2.GRUPPO4.Dashboard.Controllers
{
    [Route("api/[controller]")]
    [ApiController]
    public class DataController : ControllerBase
    {
        private readonly ICosmosDbService _cosmosDbService;

        public DataController(ICosmosDbService cosmosDbService)
        {
            _cosmosDbService = cosmosDbService ?? throw new ArgumentNullException(nameof(cosmosDbService));
        }

        // GET api/items
        [HttpGet]
        public async Task<IActionResult> List()
        {
            return Ok(await _cosmosDbService.GetMultipleAsync("SELECT top 1 * FROM c order by c._ts desc"));
        }

        // GET api/items/5
        [HttpGet("{id}")]
        public async Task<IActionResult> Get(string id)
        {
            return Ok(await _cosmosDbService.GetAsync(id));
        }

        // POST api/items
        //[HttpPost]
        //public async Task<IActionResult> Create([FromBody] Item item)
        //{
        //    item.Id = Guid.NewGuid().ToString();
        //    await _cosmosDbService.AddAsync(item);
        //    return CreatedAtAction(nameof(Get), new { id = item.Id }, item);
        //}

        // PUT api/items/5
        //[HttpPut("{id}")]
        //public async Task<IActionResult> Edit([FromBody] Item item)
        //{
        //    await _cosmosDbService.UpdateAsync(item.Id, item);
        //    return NoContent();
        //}

        // DELETE api/items/5
        [HttpDelete("{id}")]
        public async Task<IActionResult> Delete(string id)
        {
            await _cosmosDbService.DeleteAsync(id);
            return NoContent();
        }
    }
}
