Set fso = CreateObject("Scripting.FileSystemObject")
Set WshShell = CreateObject("WScript.Shell")
Dim s, infile, outfile, command

Function CreateFolderIfNonexistent(folder)
	if not fso.FolderExists(folder) then
		fso.CreateFolder(folder)
	end if
end Function

'check for existence of msgfmt
if not fso.FileExists("msgfmt.exe") then
	WScript.echo "msgfmt.exe not found. Please get gettext for windows from http://gnuwin32.sourceforge.net/packages/gettext.htm and copy the files msgfmt.exe and all dll files from the gettext/bin directory into this directory and run this script again."
	WScript.Quit(1)
end if

CreateFolderIfNonexistent("..\..\translations\")

'open translation list
Set stream = fso.GetFile("..\..\po\LINGUAS").OpenAsTextStream(1, 0)

'iterate contents of translations file
translations = 0
do while not stream.AtEndOfStream
	s = stream.ReadLine()
	if inStr(s, "#") = false then
		'create the output directory
		CreateFolderIfNonexistent("..\..\translations\" + s)
		CreateFolderIfNonexistent("..\..\translations\" + s + "\LC_MESSAGES")
		'build the translate command
		infile = "../../po/" + s + ".po"
		outfile = "../../translations/" + s +"/LC_MESSAGES/tmw.mo"
		command = "msgfmt -c -o " + outfile + " " + infile
		'execute translate command
		errval = WshShell.run(command, 0, true)
		if errval = 0 then
			translations = translations + 1
		end if
	end if
loop

WScript.echo translations, " translations compiled."