# STHERM Update

- Add partial update: 
  - find needed files for update from build directory
  - Use gzip to compress all needed files for updating (create .gz files)  e.g., `gzip appStherm`
  - Use zip to compress all .gz files as a single file. e.g., `zip update.zip appStherm.gz ...`
  - upload to server (Tony@fileserver.nuvehvac.com   pass: zIWIRvgwPd)
  - upload files in the server in directory: '/usr/share/nginx/html/files'
 e.g., `scp update.zip Tony@fileserver.nuvehvac.com:/usr/share/nginx/html/files/updateV0.2.0.zip`
  - Add some information into the '/usr/share/nginx/html/updateInfo.json' file in the server: 
    * get md5 from here:  https://emn178.github.io/online-tools/md5_checksum.html
    * get the file size in bytes using `ls -l`
        - CurrentFileSize is the final zip file size
        - RequiredMemory is the sum of uncompressed binary update files size
    * update the file address based on name you copied
    * update release date
    * for the changeLog acknowledge the markdown format
    * Every version should follow the major.minor.patch format, encompassing major, minor, and patch components (ex: "0.1.5").
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
- Staging Update
as mentioned in the preious section, there is a `staging` variable in the json, which you can set to true in order to stage an update.
staged update is available only in test mode; i.e., only when ti not attached or user activated it by pressing the FCC ID in Device Info
default OFF if you omit this

- Force Update
Similar to Staging, there is a `ForceUpdate` variable in the json, which you can set to true in order to Force an update.
Force update will be installed without user intervention in the background as soon as it is vailable
default OFF if you omit this

# OBSOLETE
these section is handled in the software and not needed anymore
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
- Make sure the 'update.sh' exist in the app directory.
