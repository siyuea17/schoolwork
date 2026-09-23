Set ws = CreateObject("WScript.Shell")
lnk = ws.CreateShortcut("C:\Users\siyuea\OneDrive\文档\学习工作（大学）杂物\王新疆-20251003860\连连看.lnk")
lnk.TargetPath = ".\连连看\连连看.exe"
lnk.WorkingDirectory = ".\连连看"
lnk.Description = "连连看"
lnk.Save()
