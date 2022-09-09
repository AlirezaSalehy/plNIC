using Microsoft.Extensions.Logging;
using Pastel;
using System.Drawing;
using System.Runtime.CompilerServices;

namespace plNICDriver
{
	public static class LoggerExtension
	{
		private static object _consoleLock = new object();

		public static void LCritical(this ILogger logger, string? message, 
									[CallerMemberName] string callerMemberName = "", [CallerFilePath] string callerPathName = "")
			=> Log("Crit".Pastel(Color.Red), message ?? "", callerMemberName, callerPathName);

		public static void LError(this ILogger logger, string? message, 
							[CallerMemberName] string callerMemberName = "", [CallerFilePath] string callerPathName = "")
			=> Log("Eror".Pastel(Color.DarkRed), message ?? "", callerMemberName, callerPathName);

		public static void LWarning(this ILogger logger, string? message,
									[CallerMemberName] string callerMemberName = "", [CallerFilePath] string callerPathName = "")
			=> Log("Warn".Pastel(Color.Yellow), message ?? "", callerMemberName, callerPathName);

		public static void LInformation(this ILogger logger, string? message,
									[CallerMemberName] string callerMemberName = "", [CallerFilePath] string callerPathName = "")
			=> Log("Info".Pastel(Color.DarkGreen), message ?? "", callerMemberName, callerPathName);

		public static void LDebug(this ILogger logger, string? message, 
									[CallerMemberName] string callerMemberName = "", [CallerFilePath] string callerPathName = "")
			=> Log("Dbug", message ?? "", callerMemberName, callerPathName);

		private static string LogFormatter(string level, string message, string memberName, string pathName)
		{
			var pathSplitted = pathName.Split('\\');
			var className = pathSplitted.LastOrDefault("");
			return $"{level}: {message} at {memberName.Pastel(Color.DarkOrange)} in {className.Pastel(Color.DarkKhaki)}";
		}

		private static string AddTime(string log)
		{
			var timeStamp = DateTime.Now.ToString("HH:mm:ss");
			return $"{timeStamp.Pastel(Color.DarkCyan)} {log}";
		}

		private static void Log(string level, string message, string memberName, string pathName)
		{
			var logMsg = LogFormatter(level, message, memberName, pathName);
			var tmdLogMsg = AddTime(logMsg);
			lock (_consoleLock)
				Console.WriteLine(tmdLogMsg);
		}
	}

	public static class ByteExtension
	{
		public static string ToStr(this byte[] bytes)
			=> PrintByteArray(bytes, bytes.Length);

		public static string ToStr(this byte[] bytes, int toLen)
			=> PrintByteArray(bytes, toLen);

		private static string PrintByteArray(byte[] bytes, int len)
		{
			var toWhere = len > bytes.Length ? bytes.Length : len;
			var sb = new System.Text.StringBuilder("byte[] { ");
			for (int i = 0; i < toWhere; i++)
			{
				sb.Append(bytes[i] + ", ");
			}
			sb.Append("}");

			return sb.ToString().Pastel(Color.BurlyWood);
		}
	}
}
