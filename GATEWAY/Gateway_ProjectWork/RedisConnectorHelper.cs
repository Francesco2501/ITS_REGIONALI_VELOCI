using StackExchange.Redis;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Gateway_ProjectWork
{
    public class RedisConnectorHelper
    {
        /*
         * ### --- Redis on windows --- ###
         * 
         * 1) Use WSL2
         * 2) ->    curl -fsSL https://packages.redis.io/gpg | sudo gpg --dearmor -o /usr/share/keyrings/redis-archive-keyring.gpg
              ->    echo "deb [signed-by=/usr/share/keyrings/redis-archive-keyring.gpg] https://packages.redis.io/deb $(lsb_release -cs) main" | sudo tee /etc/apt/sources.list.d/redis.list
              ->    sudo apt-get update
              ->    sudo apt-get install redis
         *
         * 3) ->    sudo service redis-server start
         */

        static RedisConnectorHelper()
        {
            RedisConnectorHelper.lazyConnection = new Lazy<ConnectionMultiplexer>(() =>
            {
                return ConnectionMultiplexer.Connect("localhost:6379,allowAdmin=true");
            });
        }

        private static Lazy<ConnectionMultiplexer> lazyConnection;

        public static ConnectionMultiplexer Connection
        {
            get
            {
                return lazyConnection.Value;
            }
        }

        public static void RemoveAllKeys()
        {
            lazyConnection.Value.GetServer("localhost", 6379).FlushAllDatabases();
        }
        public static void RemoveKey(string key)
        {
            lazyConnection.Value.GetDatabase().KeyDelete(key);
        }

        public static List<string> GetAllKeys()
        {
            List<string> listKeys = new List<string>();

            var keys = lazyConnection.Value.GetServer("localhost", 6379).Keys();
            listKeys.AddRange(keys.Select(key => (string)key).ToList());

            return listKeys;
        }
    }
}
