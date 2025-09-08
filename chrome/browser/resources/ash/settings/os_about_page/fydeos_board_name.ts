// append new board name here

export const FydeOSBoardNameTitleMap: {[index: string]: string} = {
  'itnt-72': 'itNT 72',
  'itnt-72-go': 'itNT 72 GO',
  'itnt-x': 'itNT X',
  'itnt-80-go': 'itNT 80 GO',
  'link_fydeos': 'FydeOS Link',
  'samus_fydeos': 'FydeOS Samus',
  'jerry-fydeos': 'FydeOS Jerry',
  'fizz-fydeos': 'Fyderbox',
  'magicbook': 'MagicBook',
  'gpdpocket2': 'GPD Pocket 2',
  'pinebook-pro': 'Pinebook Pro',
  'surface-pro3': 'Surface Pro 3',
  'surface-pro4': 'Surface Pro 4',
  'surface-pro5': 'Surface Pro 5',
  'surface-pro6': 'Surface Pro 6',
  'surface-pro7': 'Surface Pro 7',
  'surface-pro7p': 'Surface Pro 7+',
  'surface-pro8': 'Surface Pro 8',
  'surface-go': 'Surface GO',
  'surface-go2': 'Surface GO 2',
  'surface-go3': 'Surface GO 3',
  'eve_fydeos': 'Pixelbook',
  'kukui_fydeos': 'Fydetab Duo',
  'one-a1': 'One A1',
  'rpi4-fydeos': 'Raspberry Pi 400',
  'rpi5-fydeos': 'Raspberry Pi 5',
  'orangepi5-fydeos': 'Orange Pi 5',
  'rock5b-fydeos': 'Rock 5B',
  'fydetab_duo-fydeos': 'Fydetab Duo',
};

export const FydeOSBoardNameTitleListWithI18n = [{
  board: 'amd64-fydeos',
  key: 'aboutFydeOSDeviceTitleLegacyIntel',
  fallback: 'Legacy Intel',
}, {
  board: 'amd64-fydeos_iris',
  key: 'aboutFydeOSDeviceTitleModernIntel',
  fallback: 'Modern Intel',
}, {
  board: 'amd64-fydeos_apu',
  key: 'aboutFydeOSDeviceTitleAMDGraphics',
  fallback: 'AMD Graphics',
}, {
  board: 'amd64-fydeos_slim',
  key: 'aboutFydeOSDeviceTitleIntelSlim',
  fallback: 'Intel Slim',
}];

export const FydeOSBoardNameReleaseNameMap: {[index: string]: string} = {
  'amd64-fydeos': 'for PC',
  'amd64-fydeos_iris': 'for PC',
  'amd64-fydeos_apu': 'for PC',
  'amd64-fydeos_slim': 'for PC',
  'amd64-vmware': 'for VMware',
  'rpi4-fydeos': 'for SBC',
  'rpi5-fydeos': 'for SBC',
  'rock5b-fydeos': 'for SBC',
  'orangepi5-fydeos': 'for SBC',
  'fydetab_duo-fydeos': '-',
}
