# STHERM Update

- Add partial update: 
  - Use gzip to compress all needed files for updating (create .gz files) 
  - Use zip to compress all .gz files as a single file.
  - connect to server (ssh Tony@fileserver.nuvehvac.com   pass: zIWIRvgwPd)
  - upload files in the server in directory: '/usr/share/nginx/html/files'
  - Add some information into the update.json file in the server: 
 ```
"LatestVersion" : "0.1.5",
"0.1.5": {
		"CurrentFileSize": 5715000, // downloadable file size
		"RequiredMemory": 14785850, // size of files after completely decompressed.
		"checkSum": "ffa2ad90c16ed65bb1cae39255fdadbe",
		"address": "/files/update.v0.1.5.zip", // Relative path to upload update file
		"releaseDate": "20/12/2023", // Relase data with proper format
		"changeLog": "- V1.5 update added. \n - update system added"  // Change logs
	}
```
- Stablish an update service on the device using the following settings: extract update files from a zip-formatted file, decompress .gz files into actual files, transfer uncompressed files to the 'appStherm' directory, and initiate the 'appStherm' service.:
```
[Unit]
Nuve Smart HVAC system update service
[Service]
Type=simple
ExecStart=/bin/bash -c "/usr/local/bin/update.sh"
Restart=on-failure
[Install]
WantedBy=multi-user.target
```
- MAke sure the 'update.sh' exist in the app directory.