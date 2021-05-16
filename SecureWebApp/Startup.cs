using Microsoft.AspNetCore.Builder;
using Microsoft.AspNetCore.Hosting;
using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Identity;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.DependencyInjection.Extensions;
using Microsoft.Extensions.Hosting;
using SecureWebApp.Data;
using SecureWebApp.Interfaces;
using SecureWebApp.Services;
using System;

namespace SecureWebApp
{
    public class Startup
    {
        public Startup(IConfiguration configuration)
        {
            Configuration = configuration;
        }

        public IConfiguration Configuration { get; }
        
        /// <summary>
        /// Service resolver for different implementations of IBreachService
        /// Returns concrete service based on given key stored in the service class
        /// </summary>
        /// <param name="key">The name of the service class to be used.</param>
        /// <returns></returns>
        public delegate IBreachCheckService BreachServiceResolver(string key);
        
        // This method gets called by the runtime. Use this method to add services to the container.
        public void ConfigureServices(IServiceCollection services)
        {
            services.AddDbContext<ApplicationDbContext>(options =>
                options.UseSqlServer(
                    Configuration.GetConnectionString("DefaultConnection")));

            services.AddDatabaseDeveloperPageExceptionFilter();

            services.AddTransient<IMailService, EmailSenderService>();

            services.AddDefaultIdentity<IdentityUser>(options =>
                {
                    const int pwLen = 16;

                    options.SignIn.RequireConfirmedAccount = true;
                    options.SignIn.RequireConfirmedEmail = true;
                    options.SignIn.RequireConfirmedPhoneNumber = false;

                    options.Password.RequireDigit = true;
                    options.Password.RequireLowercase = true;
                    options.Password.RequireUppercase = true;
                    options.Password.RequireNonAlphanumeric = true;
                    options.Password.RequiredLength = pwLen;
                    options.Password.RequiredUniqueChars = pwLen / 3; // min. 33% uniqueness

                    options.Lockout.AllowedForNewUsers = true;
                    options.Lockout.DefaultLockoutTimeSpan = TimeSpan.FromHours(24);
                    options.Lockout.MaxFailedAccessAttempts = 5;

                    options.Tokens.ProviderMap.Add("AccountUnlockTokenProvder", new TokenProviderDescriptor(typeof(EmailTokenProvider<IdentityUser>)));
                })
                .AddEntityFrameworkStores<ApplicationDbContext>();

            services.AddHttpClient(PwnedApiCheckService.Name, c =>
            {
                c.BaseAddress = new Uri("https://api.pwnedpasswords.com");
                c.DefaultRequestVersion = new Version(2, 0);
                c.Timeout = TimeSpan.FromSeconds(5);
                c.DefaultRequestHeaders.Add("Accept", "text/plain");
                c.DefaultRequestHeaders.Add("UserAgent", Configuration["HttpUserAgent"]);
            });

            services.TryAddSingleton<IHttpContextAccessor, HttpContextAccessor>();
            services.AddTransient<IIpAddressService, IpAddressService>();

            services.AddTransient<PwnedApiCheckService>();
            services.AddTransient<PasswdsApiCheckService>();

            services.AddTransient<BreachServiceResolver>(provider => key =>
            {
                return key switch
                {
                    PwnedApiCheckService.Name => provider.GetService<PwnedApiCheckService>(),
                    PasswdsApiCheckService.Name => provider.GetService<PasswdsApiCheckService>(),
                    _ => throw new ArgumentException("Unknown service name provided.", nameof(key))
                };
            });
            
            services.AddControllersWithViews();
        }

        // This method gets called by the runtime. Use this method to configure the HTTP request pipeline.
        public void Configure(IApplicationBuilder app, IWebHostEnvironment env)
        {
            if (env.IsDevelopment())
            {
                app.UseDeveloperExceptionPage();
                app.UseMigrationsEndPoint();
            }
            else
            {
                app.UseExceptionHandler("/Home/Error");
                // The default HSTS value is 30 days. You may want to change this for production scenarios, see https://aka.ms/aspnetcore-hsts.
                app.UseHsts();
            }
            app.UseHttpsRedirection();
            app.UseStaticFiles();

            app.UseRouting();

            app.UseAuthentication();
            app.UseAuthorization();

            app.UseEndpoints(endpoints =>
            {
                endpoints.MapControllerRoute(
                    name: "default",
                    pattern: "{controller=Home}/{action=Index}/{id?}");
                endpoints.MapRazorPages();
            });
        }
    }
}
