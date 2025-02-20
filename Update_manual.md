# STHERM Manual Update

- Add manual partial update: 
  - find needed files for update from build directory
  - Use gzip to compress all needed files for updating (create .gz files)  e.g., `gzip appStherm`
  - Use zip to compress all .gz files as a single file. e.g., `zip update.zip appStherm.gz ...`
  - upload to server (Tony@fileserver.nuvehvac.com   pass: zIWIRvgwPd)
  - upload files in the server in directory: '/usr/share/nginx/html/manual_update'
 e.g., `scp update.zip Tony@fileserver.nuvehvac.com:/usr/share/nginx/html/manual_update/testFile.zip`
  - Add some information into the '/usr/share/nginx/html/manual_update/files_info.json' file in the server: 
	* get md5 from here:  https://emn178.github.io/online-tools/md5_checksum.html
	* get the file size in bytes using `ls -l`, 
		- CurrentFileSize is the final zip file size
  		- RequiredMemory is the sum of uncompressed binary update files size
	* for the changeLog acknowledge the markdown format and write simple description for user to see
 ```
"testFile.zip": {
        "CurrentFileSize": 6116653,
        "RequiredMemory": 15907616,
        "CheckSum": "24bd0a8429ad2f54260cf9350f20a73d",
        "ChangeLog": " - Sync to server added. \n - nRF Firmware updated to 01.10. \n - Some UI bug fixes & Improvements.\n - backdoor test file."
    }
```
