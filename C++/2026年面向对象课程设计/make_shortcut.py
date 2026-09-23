import os, pythoncom
from win32com.client import Dispatch

target = r"C:\Users\siyuea\OneDrive\文档\学习工作（大学）杂物\王新疆-20251003860"
shell = Dispatch("WScript.Shell")
sc = shell.CreateShortcut(os.path.join(target, "连连看.lnk"))
sc.TargetPath = r".\连连看\连连看.exe"
sc.WorkingDirectory = r".\连连看"
sc.Description = "连连看"
sc.Save()
print("Shortcut created.")
