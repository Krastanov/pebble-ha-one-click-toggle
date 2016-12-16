module.exports = [
  {
    "type": "heading",
    "defaultValue": "HA Configuration"
  },
  {
    "type": "text",
    "defaultValue": "You need to provide URL and password."
  },
  {
    "type": "section",
    "items": [
      {
        "type": "input",
        "messageKey": "HAurl",
        "defaultValue": "",
        "label": "URL"
      },
      {
        "type": "input",
        "messageKey": "HApwd",
        "defaultValue": "",
        "label": "password"
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];
