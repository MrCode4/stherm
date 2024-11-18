
Release Note
-------------
## Release R 1.4.4
 15th November 2024
 Branch- r1_4_4_2, commit- 12d33a9f
 
### Release Articles
update-V.1.4.4.2.zip

### Reason
Add new features and fix some issues.

### What’s Brand New
- Implemented sensor data transmission in Protobuf format.

### What Been Updated
- Updated heat pump system type in both the user interface and backend logic.
- Updated dual fuel system type in both the user interface and backend logic.
- Adjust logo field.
- Update temperature scheme logics based on the new version of scheme block diagram (V2.0.2)

### What’s Been Fixed
- Fix temperature scheme bugs.

### Known Issues
- CPU usage in some cases should be check.

## Release R 1.4.3
 4th Novemeber 2024
 Branch- r1_4_3, commit- 9e211cac39
 
### Release Articles
update-V.1.4.3.zip

### Reason
Fix Performance test and update some text

### What’s Brand New
- Duplicate option for schedules on type error

### What Been Updated
- Text of the error for schedules type error

### What’s Been Fixed
- Device shown offline during performance test
- Schedules activating during performance test
- Not Showing some certaing bullets in messages and etc.

### Known Issues
- CPU usage in some cases should be check.

## Release R 1.4.2
 31st October 2024
 Branch- r1_4_2, commit- 3375189532
 
### Release Articles
update-V.1.4.2.zip

### Reason
Update Schedule and fix some issues

### What’s Brand New
- Skipping when Job id has error from error page.

### What Been Updated
- UI and underlying logic for schedules redesigned.

### What’s Been Fixed
- Some issues regarding performance test
- Some issues regarding first run flow
- Pop up orders during factory registration

### Known Issues
- CPU usage in some cases should be check.
- Performance test hase some issues.

## Release R 1.4.1
 29th October 2024
 Branch- r1_4_1, commit- 81d0cd22
 
### Release Articles
update-V.1.4.1.zip

### Reason
Add new features and fix some issues

### What’s Brand New
- Added a confirmation message when clicking x on add schedule wizard.
- Added confirmation on delete schedule.
- Added "Contact Nuve Support: (657) 626-4887 for issues." on specific first run flow pages.
- Added performance test.

### What Been Updated
- Show all errors in the first run flow requests.

### What’s Been Fixed
- Fixed some issues.

### Known Issues
- CPU usage in some cases should be check.
- Performance test hase some issues.

## Release R 1.4.0
 26th October 2024
 Branch- r1_4_0, commit- d81ff8bc
 
### Release Articles
update-V.1.4.0.zip

### Reason
Add new feature and fix some issues

### What’s Brand New
- Added some CMAKE options for developer tests.
- Added gradient to menus like main and settings menu, messages, alert and system alert lists.
- Added Lock/Unlock API.

### What Been Updated
- Implemented the new UI design for server messages.

### What’s Been Fixed
- Fixed showing notification dot in the main menu.

### Known Issues
- CPU usage in some cases should be check.

## Release R 1.3.1
 9th October 2024
 Branch- r1_3_1, commit- 15c6fb9c
 
### Release Articles
update-V.1.3.1.zip

### Reason
Add new feature and fix some issues

### What’s Brand New
- Send test results to the specified server in test mode.

### What Been Updated
- Sync dual fuel threshold with the server.

### What’s Been Fixed
- Fixed dual fuel UI issue in the first run flow.
- The user interface of the main and settings menus was polished.
- Fixed the registration flow bugs.

### Known Issues
- CPU usage in some cases should be check.

## Release R 1.3.0
 5th October 2024
 Branch- r1_3_0, commit- d1b02075b0
 
### Release Articles
update-V.1.3.0.zip

### Reason
Add new features and fix some issues specifically schedules and dual fuel heating.

### What’s Brand New
- Added dual fuel heating.
- Added UI to manage the main endpoint in test mode.
- Added icon to display if wifi has password or not.

### What Been Updated
- Schedules sync with the server with new APIs using new APIs for adding, editing, and deleting schedules..
- Display message creation times in local time. .
- Limit the number of stored messages to 50 to optimize performance.
- Update messages id with the server.
- Mobile app page updated with the new QR code based on the serial number.
- Send log when SN fetching fails and show fetch SN retry in wifi page in first run flow. 

### What’s Been Fixed
- Fixed schedules issues.
- Fixed message comparison logic to avoid duplicates.
- Fixed some wifi issues.
- Fixed temperature unit in the scheme data provider to stop cooling in the correct way.
- Fixed start/stop schedule running issue.

### Known Issues
- Push dueal fuel threshold to server and fetch it from server.
- CPU usage in some cases should be check.

## Release R 1.2.1.1
 26th September 2024
 Branch- r1_2_1, commit- 6d5a44614
 
### Release Articles
update-V.1.2.1.zip

### Reason
Add new features and fix some issues Specifically first run flow.

### What’s Brand New
- Simple Stack View implemented and used in tests.
- fetched client data from server.
- Restructured the menu.
- Redesigned the initial run flow.
- Added warranty replacement.

### What Been Updated
- Hide sensor values until actual values fetch from sensors.
- Access to wifi settings when no internet in lock screen.
- Update mobile app page.
- Remove Nuve icon from default of ContactContractor model.
- Check sensors health and manage the schemes.
- Update the schedule UI and logics.
- Factory test flow up check for update and sshpass on tests. 

### What’s Been Fixed
- Fixed save sequence for fetched data.
- Fixed device settings lost on startup.
- Fixed default values for null.
- Fixed GetInfo retry for TI.
- Fixed server data fetch.
- Fixed repository issues for save/load.
- Fixed clamp issue in auto and vacation mode, updated model on data change.
- Fixed overlap, enable/disable, update from server for schedules.
- Fixed some WIFI issues.

### Known Issues
- Schedule has some minor issues to be determined.


## Release R 1.1.1
 10th August 2024
 Branch- hotfix, commit- 57d28d0f
 
### Release Articles
update-V.1.1.1.zip

### Reason
Hotfix for regression and backdoor for lock

### What’s Brand New

### What Been Updated
- Master password added for lock screen when forget password.
- Access to wifi settings when no internet in lock screen.

### What’s Been Fixed
- Regressing with show home on unlocking in startup

### Known Issues
 - Device settings are lost on startup without WIFI.


## Release R 1.1.0
 7th August 2024
 Branch- release, commit- ae8f5fe7
 
### Release Articles
update-V.1.1.0.zip (deprecated)

### Reason
Add new features and fix some issues

### What’s Brand New
- mobile app link page.
- screen lock/unlock page.

### What Been Updated
- Alerts based on the documents.
- Improved sync flow to fix issues.
- Serial number flow improved when device is registered.
- Label of Notifications changed to Messages in settings.
- Adaptive brightness can not be turned on from server.
- Improve TOF requests flow to prevent too much data.
- Improve system mode button position and temperature labels position and size of them.
- Technician phone number updated.

### What’s Been Fixed
- Installs the sshpass if not exists.
- Time format for messages.
- All API calls are refactored in Sync.
- uuid of save file back to random to prevent new structures issues.
- Clear the cache when internet has issues.


### Known Issues
 - Device settings are lost on startup without WIFI.
 - Regression with Initial setup and test mode skipping to home screen

## Release R 1.0.2
 11th July 2024  
 Branch- master, commit- 743f30f5
 
### Release Articles
update-V.1.0.2.zip

### Reason
Fix some issues

### What’s Brand New

### What Been Updated
- fan performance a little less smooth but way less cpu usage
- Update created time of messages based on new API
- Improved ui for privacy policy page and saving timestamp of acceptance

### What’s Been Fixed
- fixing bug with vacation mode which mobile version can not resume to normal
- fix for out of sync values on mode change using validation of data upon receiving from server or before sending to it
- fix with delay on ending vacation mode

### Known Issues
 - Alerts should be updated according the docs

## Release R 1.0.0 
 1st July 2024  
 Branch- master, commit- *
 
### Release Articles
update-V1.0.0.zip

### Reason
New features

### What’s Brand New
- Implement discrete firmware version update
- Add privacy policy and terms of use in test mode and initial setup

### What Been Updated
- New update path for public releases

### What’s Been Fixed


### Known Issues
 - Alerts should be updated according the docs


## Release R 0.9.5 
 29th June 2024  
 Branch- master, commit- b3e5394
 
### Release Articles
manual(update-V0.9.4.2.zip)

### Reason
Refactor and fix some issues

### What’s Brand New
- refactor nmcli interface and api-calls\

### What Been Updated

### What’s Been Fixed
-  fix bug that could cause device to factory restore.

### Known Issues

## Release R 0.9.3 
 18th June 2024  
 Branch- master, commit- a18d81d6
 
### Release Articles
update-V0.9.3.zip

### Reason
Add new features and fix wifi critical issue

### What’s Brand New
- Muting Alerts/Notifications added.
- Alerts/Notifications and update will show icon on the Screen saver. 

### What Been Updated
- Wi-Fi improvements inlcuding showing saved networks.

### What’s Been Fixed
- Minor fixes and improvements.

### Known Issues


## Release R 0.9.2 
 10th June 2024  
 Branch- master, commit- 49593f6e
 
### Release Articles
update-V0.9.2.zip

### Reason
Add new feature and modify UX

### What’s Brand New
- Toast notifications added for schedule activations.

### What Been Updated
- Auto mode api updated.
- Better UX for sending logs.

### What’s Been Fixed
- Minor fixes and improvements.

### Known Issues

## Release R 0.9.1
9th June 2024  
Branch- master, commit- ffdc108

### Release Articles
-

### Reason
Minor fixes

### What’s Brand New

### What Been Updated

### What’s Been Fixed
- fixed some conflicts in the test sequence.
- Try to create folder at least once before sending log

### Known Issues


## Release R 0.9.0
8th June 2024  
Branch- master, commit- 926364a

### Release Articles
-

### Reason
Fix UI bugs

### What’s Brand New

### What Been Updated

### What’s Been Fixed
- Hide decimal numbers in ScheduleTempraturePage.
- Fix auto UI bug when schedule is running.
- Set time - Minutes for Time  should be 00, 01,02, 03 ... 09.
- Fix test sequences.
- Minor fixes and improvements.

### Known Issues


## Release R 0.4.6 
 28th May 2024  
 Branch- master, commit- 5cb68242
 
### Release Articles
update-V0.4.6.zip

### Reason
Some Critical fixes for old devices

### What’s Brand New
- Time and date for messages and alerts
- System delay count down and indicator.
- Info icon on initial setup flow.
- Fan animation added when it is ON.

### What Been Updated
- Update checking disabled during quiet mode
- Refreshing sn and QR url for fresh devices

### What’s Been Fixed
- Log issues with sshpass fixed
- Off mode issues fixed

### Known Issues
- Changing schedule or temperature during system delay countdown has no immediate effect


## Release R 0.4.5
 17th May 2024
Branch- master, commit- 0460796

### Release Articles
update-V0.4.5.zip

### Reason
- Fix for force update

### What’s Brand New

### What Been Updated
- Sync from server with time of last change

### What’s Been Fixed
- Force update flow
- Wrong Wi-Fi password alert

### Known Issues


## Release R 0.4.3
9th May 2024
Branch- master, commit- 1c5f5bd6

### Release Articles
update-V0.4.3.zip

### Reason
- Api for detecting last setting update time added.

### What’s Brand New
- Last settings update time is a new feature that affects fetching from the server.

### What Been Updated

### What’s Been Fixed

### Known Issues
- NRF update and normal update has some interruptions


## Release R 0.4.2
7th May 2024
Branch- master, commit- b01859e

### Release Articles
update-V0.4.2.zip

### Reason
- HVAC algorithm updated and fix some minor bugs.

### What’s Brand New
- Last settings update time is a new feature that affects fetching from the server.

### What Been Updated
- HVAC algorithm updated to V1.8.

### What’s Been Fixed
- Disable checking internet after a wrong password.
- Fix Wi-Fi wrong password alert issue
- Save messages locally as soon as they received
- Fix multiple Wi-Fi/Internet alert issue

### Known Issues



## Release R 0.4.1
2nd May 2024
Branch- master, commit- 0a62fd5e

### Release Articles
update-V0.4.1.zip

### Reason
- Fix some bugs.

### What’s Brand New

### What Been Updated

### What’s Been Fixed
- log the actual backlight rather than model data.
- Fix slider issue in auto mode.
- Fix NRF test.
- Fix vacation UI.
- Fix alerts based on new logic
- Fix ComboBox item to prevent visual issues 

### Known Issues


## Release R 0.4.0
28th April 2024
Branch- master, commit- b9308ddb

### Release Articles
update-V0.4.0.zip

### Reason
- New feature and fix some issues

### What’s Brand New
- Forget device added.

### What Been Updated
- Initial setup flow improved while there is an update.
- Alerts Ux improved according to feedback.

### What’s Been Fixed
- Log authentication issues fixed.
- System Delay interruption issue fixed with optimizing the flow.
- Auto mode labels fixed.
- Test mode buttons redesigned.
- Wi-Fi System redesigned to improve the stability.
- Fix no Internet issue.
- Fix order of alerts.


### Known Issues


## Release R 0.3.14
24th April 2024
Branch- master, commit- 1d621cce

### Release Articles
update.V0.3.14.zip

### Reason
- New feature and fix some issues

### What’s Brand New
- Notifications from contractor added.
- Alert rules implemented per documentations.
- Auto mode added.
- different ranges added for cooling/heating/auto modes.
- 'No repeat' option added to schedules.
- Easier manual update added by holding 'System update' in menu for 10 seconds.
- Exit from manual update mode added.
- Send log option added in Device Information page.
- Schedule button in home updated when there is at least one enabled schedule.
- Highlight unread messages.

### What Been Updated
- Syncing to Server has multiple improvements.
- Format of Date updated (e.g. 24 Apr 2024).

### What’s Been Fixed
- Adaptive brightness will update smoothly.
- Some schedule overlapping bugs fixed.
- Some bugs in relays controller fixed.

### Known Issues


## Release R 0.3.13
7th April 2024
Branch- master, commit- 80ed8e2b

### Release Articles
updateV0.3.13.zip

### Reason
- New feature

### What’s Brand New
- IPv4 address added to device informations.
- Sync to mobile application optimized.

### What Been Updated

### What’s Been Fixed

### Known Issues


## Release R 0.3.12
5th April 2024
Branch- master, commit- 68f8a985

### Release Articles
update.V0.3.12.zip

### Reason
- New feature and fix some issues

### What’s Brand New
- Fan colors according to actual state added.

### What Been Updated
- Checking internet connectivity made more robust.
- HVAC system hysteresis values optimized.
- Overall user experience improved

### What’s Been Fixed
- Q/A reported bugs fixed.

### Known Issues


## Release R 0.3.11
3rd April 2024
Branch- master, commit- 393a9081

### Release Articles
update.V0.3.11.zip

### Reason
- New feature and fix some minor issues.

### What’s Brand New
- Temperature compensation with fan effect.
- Add night/quiet mode.
- Add a 'backdoor' mechanism to update device in test mode.
- Add system log to write some essential system data to csv file.

### What Been Updated
- Improve device push frequency to server.

### What’s Been Fixed
- Fix download process when process is stuck for 30s or progressing very slowly.

### Known Issues

