# STHERM Update

- Add partial update: 
  - find needed files for update from build directory
  - Use gzip to compress all needed files for updating (create .gz files)  e.g., `gzip appStherm`
  - Use zip to compress all .gz files as a single file. e.g., `zip update.zip appStherm.gz ...`
  - upload to server (Tony@fileserver.nuvehvac.com   pass: zIWIRvgwPd)
  - upload files in the server in directory: '/usr/share/nginx/html/files'
 e.g., `scp update.zip Tony@fileserver.nuvehvac.com:/usr/share/nginx/html/files/updateV0.2.0.zip`
  - Add some information into the updateInfo.json file in the server: 
get md5 from here:  https://emn178.github.io/online-tools/md5_checksum.html
get the file size in bytes using `ls -l`
update the file address based on name you copied
update release data
for the changeLog acknowledge the markdown format
Every version should follow the major.minor.patch format, encompassing major, minor, and patch components (ex: "0.1.5").
 ```
"0.1.5": {
                "CurrentFileSize": 5715000, // downloadable file size
                "RequiredMemory": 14785850, // size of files after completely decompressed.
                "CheckSum": "ffa2ad90c16ed65bb1cae39255fdadbe",
                "Address": "/files/update.v0.1.5.zip", // Relative path to upload update file
                "ReleaseDate": "20/12/2023", // Relase data with proper format
                "ChangeLog": "- V1.5 update added. \n - update system added"  // Change logs
				"Staging": true, // This update available just in test mode
                "ForceUpdate": false // Forced update.  So a device receiving this will update the device without user intervention.
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
