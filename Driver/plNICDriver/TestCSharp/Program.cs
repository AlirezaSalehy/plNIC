// See https://aka.ms/new-console-template for more information


int count = 0;
//for (int i = 10 - 1; i >= 0; i--)
//	await print(i);

byte[] arr = new byte[] { 1, 2, 3, 4, 5, 6, 7, 8 };
var a = 0..3;
var b = arr[a];
Console.WriteLine(arr[0..3]);

Console.WriteLine("Hello, World!");

async Task<double> getDouble()
{
	Random random = new Random((int)DateTime.UtcNow.Ticks);
	await Task.Delay(random.Next(5000));
	Console.WriteLine(count++);
	return random.NextDouble();
}

async Task<bool> print(int i)
{
	var rez = await getDouble().WaitAsync(TimeSpan.FromMilliseconds(10000));
	Console.WriteLine($"This double number {rez} {i}");
	return true;
}

Console.WriteLine("Done!");
Console.ReadLine();