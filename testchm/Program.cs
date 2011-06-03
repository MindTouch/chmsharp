using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

using CHMsharp;

namespace testchm
{
    class Program
    {
        static string _outdir;

        static void Main(string[] args)
        {
            if (args.Length != 2) {
                Console.WriteLine("USAGE: testchm.exe <input file> <output directory>");
                return;
            }

            _outdir = args[1];

            chmFileM chmf = chmFileM.Open(args[0]);
            chmf.Enumerate(
                chmFileM.Level.All,
                new chmFileM.chmEnumerator(EnumeratorCallback),
                null);
            chmf.Close();
        }

        static chmFileM.Status EnumeratorCallback(chmFileM file, chmUnitInfoM ui, Object context)
        {
            if (!ui.path.EndsWith("/"))
                Console.WriteLine(file.FileName + ": " + ui.path);

            if (ui.length > 0) {
                byte[] buf = new byte[ui.length];
                UInt64 ret = file.RetrieveObject(ui, ref buf, 0, (UInt64)buf.Length);

                if (ret > 0) {
                    try {
                        FileInfo fi =
                            new FileInfo(Path.Combine(_outdir, ui.path.Trim('/')));
                            
                        List<DirectoryInfo> created;

                        fi.Directory.CreateDirectory(out created);
                        File.WriteAllBytes(fi.FullName, buf);
                    } catch (ArgumentException ex) {
                        Console.WriteLine(ex.Message);
                    }
                }
            }

            return chmFileM.Status.Continue;
        }
    }
}
