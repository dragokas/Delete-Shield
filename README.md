# Delete‑Shield

**DeleteShield** is a utility that blocks deletion of newly created files.  
It can be useful for preventing **self-deleting files**, such as those created by **7z SFX scripts** or other temporary extractors.

The utility recursively monitors a specified directory.  
As soon as a new file is created, **DeleteShield immediately blocks deletion** using one of the following methods:

- Adding a `Deny` ACE to the file's NTFS permissions.
- Opening a file handle without `FILE_SHARE_DELETE`, preventing delete access.

DeleteShield can be used **interactively** or via the **command line**.

---

## Command Line Options

The utility supports three modes:

| Mode | Description |
|-----|-------------|
| `-m` | Monitor directory and display newly created files. |
| `-b` | Block delete access for newly created files. |
| `-u` | Remove ACE restrictions. |

---

## Examples

### Monitor newly created files

Recursively monitor the user's **Temp** directory and print file creation events to the console (no blocking is applied):
```
DeleteShield.exe -m "%Temp%"
```

### Block deletion of newly created files

Recursively block delete operations for all newly created files in the user's **Temp** directory:
```
DeleteShield.exe -b "%Temp%"
```

### Remove delete restrictions

Recursively remove all **Deny ACEs** from files and folders:
```
DeleteShield.exe -u "%Temp%"
```
---

## Compatiblity

 - OS Windows 7x64 or newer

---

## Security Considerations

⚠️ **Important:** Do **not** use the block mode on the entire `C:\` drive or on system folders (e.g., `C:\Windows`, `C:\Program Files`).  
Doing so may **cause system instability or a Blue Screen of Death (BSOD)**.  
Always test in a controlled environment and start with **read-only monitoring mode** before applying blocks.

---

## Typical Use Case

Example: Retrieving **Acer** BIOS firmware files from installer.

1. Download the firmware:  
   [Acer BIOS Downloads](https://www.acer.com/ru-ru/support/product-support/ANV15-51/NH.QN9CD.001/downloads)
2. Run DeleteShield in **read-only monitoring mode** to observe file creation:  
   ```cmd
   DeleteShield.exe -m "C:\"```
3. Run the firmware installer and watch the console logs to identify which folder needs protection.
4. In the logs, you possibly can see the entries like:
```C:\Windows\Temp\7zS7620.tmp\msvcp90.dll```
The folder 7zS7620.tmp is randomly generated, so the parent folder should be blocked.
5. Run DeleteShield in block mode on the parent folder:
```
DeleteShield.exe -b C:\Windows\Temp
```
6. Run the firmware installer again.
7. Copy the required files from C:\Windows\Temp.
8. Remove the blocks by running DeleteShield in unlock mode:
```
DeleteShield.exe -u C:\Windows\Temp
```
9. Clean up any unnecessary temporary folders.

---

## Building from sources

 - Microsoft Visual Studio 2026 on Windows 10 or newer is required
