// See https://aka.ms/new-console-template for more information
using plNICDriver.Link;
using plNICDriver.Phy;
using System.IO.Ports;

Console.WriteLine("Hello, World!");

byte [] buffer = new byte[1024];
getByteArr(buffer);

Console.WriteLine(buffer[0]);

foreach (var item in PHYSerial.GetAvailablePortsName())
{
	Console.WriteLine(item);
}


void getByteArr(byte[] bytes)
{
	bytes[0] = 10;
}
