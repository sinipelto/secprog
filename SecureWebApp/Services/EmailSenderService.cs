﻿using Microsoft.AspNetCore.Identity;
using Microsoft.Extensions.Logging;
using SecureWebApp.Interfaces;
using SecureWebApp.Models;
using System;
using System.Security.Policy;
using System.Text.Encodings.Web;
using System.Threading.Tasks;
using Microsoft.Extensions.Configuration;

namespace SecureWebApp.Services
{
    public class EmailSenderService : IMailService
    {
        private readonly IConfiguration _configuration;
        private readonly ILogger<EmailSenderService> _logger;
        private readonly IIpAddressService _ipAddressService;

        public EmailSenderService(
            ILogger<EmailSenderService> logger,
            IIpAddressService ipAddressService, 
            IConfiguration configuration)
        {
            _logger = logger;
            _ipAddressService = ipAddressService;
            _configuration = configuration;
        }

        public async Task SendEmailAsync(EmailMessage message)
        {
            await Task.Yield();
            _logger.LogDebug($"CREATED EMAIL:\nTO: {message.To}\nSUBJECT: {message.Subject}\nMESSAGE: {message.HtmlMessage}");
        }

        public async Task SendAccountUnlockEmailAsync(IdentityUser user, string recoveryToken)
        {
            await SendEmailAsync(new EmailMessage
            {
                To = user.Email,
                Subject = "SecureWebApp: User account locked",
                HtmlMessage = GetTemplate(Templates.AccountUnlock, user.Email, _ipAddressService.GetRequestIp(), user.Id, recoveryToken)
            });
        }


        public enum Templates { AccountUnlock }

        /// <summary>
        /// For AccountUnlock:
        /// 0: Username/Email
        /// 1: IP Address
        /// 2: userId
        /// 3: unlock token
        /// </summary>
        /// <param name="template">Template to be used</param>
        /// <param name="input">All required data parameters for the template. See the lists above.</param>
        /// <returns></returns>
        private string GetTemplate(Templates template, params string[] input)
        {
            switch (template)
            {
                // 0: Username/Email
                // 1: IP Address
                // 2: userId
                // 3: unlock token
                case Templates.AccountUnlock:
                    var url = new Uri(Uri.UriSchemeHttps + Uri.SchemeDelimiter + _configuration["ServerName"] +
                                      "/Identity/Account/UnlockAccount" + "?userId=" + input[2] + "&code=" + input[3]);
                    return
                        $"Hello, {input[0]}! Due to security reasons your account has been locked after too many failed login attempts.\n\nThis lockdown was originated from IP address {input[1]}.\n\n" +
                        "The account will automatically unlock after 24 hours from lockdown.\n\n" +
                        "<a href='" + url + "'>Unlock your account now by clicking here</a>";
                default:
                    throw new ArgumentOutOfRangeException(nameof(template), template, null);
            }
        }

        public static string GetSubject(Templates template, params string[] input)
        {
            return template switch
            {
                Templates.AccountUnlock => "SecureWebApp: User account locked",
                _ => throw new ArgumentOutOfRangeException(nameof(template), template, null)
            };
        }
    }
}