
Release Note
-------------

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

