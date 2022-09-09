// See https://aka.ms/new-console-template for more information
using Microsoft.Extensions.Logging;
using plNICDriver.Link;
using static plNICDriver.LoggerExtension;
using System.Text;
using Pastel;
using System.Drawing;
using plNICDriver.Net;

// This is a demo to show how driver library works

/**
 * TODO: 
 *  -1 plNIC AutoDetect
 *  -3 Async communication -> Done
 *  
 */

ILoggerFactory loggerFactory = LoggerFactory.Create(builder => {
	//builder.AddSimpleConsole((i) =>
	//{
	//	i.SingleLine = true;
	//	//i.IncludeScopes = false;
	//	i.ColorBehavior = Microsoft.Extensions.Logging.Console.LoggerColorBehavior.Enabled;
	//	i.TimestampFormat = "HH:mm:ss ";
	//});
	builder.SetMinimumLevel(LogLevel.Debug);
});
ILogger<Program> _lg = loggerFactory.CreateLogger<Program>();
_lg.LInformation("This is a Demo of plNIC Driver utilization");

string portName;
byte id;

bool GetArgs(out string portName, out byte id)
{
	portName = "";
	id = 0;
	if (args[0] == null || args[0].Length < 3)
	{
		_lg.LCritical("Portnumber is invalid");
		return false;
	}
	portName = args[0];

	if (args[1] == null || args[1].Length < 1)
		_lg.LWarning($"Id is not provided");
	else
		id = byte.Parse(args[1]);

	return true;
}

async void RunCMDInterface(Transport netPort)
{
	try
	{
		var res = await netPort.Begin();
		if (!res)
			return;

		_lg.LInformation("All done! can now send messages, -1 to terminate");

		SendMessages();

		netPort.Dispose();

		void SendMessages()
		{
			while (true)
			{
				var input = Console.ReadLine();
				if (input == "-1")
					break;

				if (input is null)
					continue;

				// Fragmentation??
				Task.Run(async () =>
				{
					var status = await netPort.SendSegment(((byte)(10 - id)), Encoding.ASCII.GetBytes(input));
					_lg.LInformation($"Segment Status " +
						$"{status.ToString().Pastel(Color.DarkBlue).PastelBg(Color.LightGreen)}");
				});
			}
		}
	}
	catch (Exception e)
	{
		Console.WriteLine(e);
	}


}

if (GetArgs(out portName, out id))
{
	Transport netPort = new Transport(loggerFactory, portName, id, (byte txid, byte[] dat) => {
		_lg.LInformation($"Recv: {Encoding.ASCII.GetString(dat).Pastel(Color.Black).PastelBg(Color.LightGreen)}");
	});

	RunCMDInterface(netPort);
}

Console.ReadLine();