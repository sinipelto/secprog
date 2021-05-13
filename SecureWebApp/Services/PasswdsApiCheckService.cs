using System.Net.Http;
using System.Security.Cryptography;
using System.Threading.Tasks;
using SecureWebApp.Interfaces;

namespace SecureWebApp.Services
{
    public class PasswdsApiCheckService: IBreachCheckService
    {
        public const string Name = "PasswdsApiService";

        private const string Path = "/range";

        private readonly IHttpClientFactory _httpClientFactory;
        
        public async Task<bool> CheckPassword(string password)
        {
            throw new System.NotImplementedException();
        }

        public bool CheckHash<T>(T hash) where T : HashAlgorithm
        {
            throw new System.NotImplementedException();
        }
    }
}