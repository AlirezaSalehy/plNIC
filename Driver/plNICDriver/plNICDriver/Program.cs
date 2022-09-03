// See https://aka.ms/new-console-template for more information
using Microsoft.Extensions.Logging;
using plNICDriver.Link;
using plNICDriver.Phy;
using System.IO.Ports;
using System.Text;

/**
 * TODO: 
 *  -1 plNIC AutoDetect
 *  -2 port support
 *  -3 Async communication
 *  
 */

try
{
	ILoggerFactory loggerFactory = LoggerFactory.Create(builder => {
		builder.AddSimpleConsole((i) =>
		{
			i.ColorBehavior = Microsoft.Extensions.Logging.Console.LoggerColorBehavior.Enabled;
			i.TimestampFormat = "HH:mm:ss ";
		});
		builder.SetMinimumLevel(LogLevel.Debug);
	});
	ILogger<Program> _lg = loggerFactory.CreateLogger<Program>();

	Console.WriteLine("This is a Demo of plNIC Driver utilization");
	var comPort = args.Length > 0 ? args[0] : GetPortName();
	if (comPort == null)
	{
		Console.WriteLine("Portnumber is invalid");
		return;
	}

	var id = int.Parse(args[1]);
	Link link = new Link(loggerFactory, id, comPort, 3, 3500, 3000, (byte txid, byte[] dat) => {
		_lg.LogInformation("Recv: {0}", Encoding.ASCII.GetString(dat));
	});

	var res = await link.Begin();
	if (!res)
		return;

	_lg.LogInformation("All done! can now send messages, -1 to terminate");

	SendPackets();

	link.Dispose();

	void SendPackets()
	{
		while (true)
		{
			var input = Console.ReadLine();
			if (input == "-1")
				break;

			if (input is null)
				continue;

			// Fragmentation??
			Task.Run(() =>
			{
				var status = link.SendPacket(Encoding.ASCII.GetBytes(input));
				Console.WriteLine($"Packet Status {status.Result}");
			});
		}
	}

	string? GetPortName()
	{
		int counter = 1;
		Console.WriteLine("Enter plNIC port to use:");
		var ports = PHYSerial.GetAvailablePortsName();
		foreach (var portName in ports)
			Console.WriteLine($"-{counter++} {portName}");
		var input = Console.ReadLine();
		int.TryParse(input, out int portNumber);
		if (portNumber >= 1 && portNumber < ports.Length)
			return ports[portNumber - 1];
		else
			return null;
	}

}
catch (Exception e)
{
	Console.WriteLine(e);
}

