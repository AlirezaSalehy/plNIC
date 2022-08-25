// See https://aka.ms/new-console-template for more information
using plNICDriver.Link;
using plNICDriver.Phy;
using System.IO.Ports;
using System.Text;

foreach (var item in PHYSerial.GetAvailablePortsName())
	Console.WriteLine(item);

Link link = new Link("COM3", 3, 1000, (byte txid, byte[] dat) => {
	Console.WriteLine("Recv: {0}", Encoding.ASCII.GetString(dat));
});
link.Begin();

string hi = "hello world!";
link.SendPacket(Encoding.ASCII.GetBytes(hi));

Console.ReadLine();
