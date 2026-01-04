function doGet(e) { 
  Logger.log(JSON.stringify(e));
  var result = 'Ok';
  
  if (e.parameter == 'undefined') {
    result = 'No Parameters';
  }
  else {
    var sheet_id = 'ID'; 
    var sheet_name = "DATA";  

    var sheet_open = SpreadsheetApp.openById(sheet_id);
    var sheet_target = sheet_open.getSheetByName(sheet_name);

    if (!sheet_target) {
      sheet_target = sheet_open.insertSheet(sheet_name);
      // Tạo header cho bảng dữ liệu
      sheet_target.getRange(1, 1, 1, 4).setValues([['Date', 'Time', 'Heart Rate (bpm)', 'SpO2 (%)']]);
      sheet_target.getRange(1, 1, 1, 4).setFontWeight('bold');
      sheet_target.setFrozenRows(1);
    }

    var rowDataLog = [];

    var Curr_Date = Utilities.formatDate(new Date(), "Asia/Ho_Chi_Minh", 'dd/MM/yyyy');
    rowDataLog[0] = Curr_Date;  // Cột A: Ngày

    var Curr_Time = Utilities.formatDate(new Date(), "Asia/Ho_Chi_Minh", 'HH:mm:ss');
    rowDataLog[1] = Curr_Time;  // Cột B: Giờ

    var sts_val = '';

    for (var param in e.parameter) {
      Logger.log('In for loop, param=' + param);
      var value = stripQuotes(e.parameter[param]);
      Logger.log(param + ':' + e.parameter[param]);
      
      switch (param) {
        case 'sts':
          sts_val = value;
          break;

        case 'hr':
          rowDataLog[2] = parseFloat(value);  
          result += ', HR Written';
          break;

        case 'spo2':
          rowDataLog[3] = parseFloat(value);  
          result += ', SpO2 Written';
          break;       

        default:
          result += ", unsupported parameter";
      }
    }
    
    if (sts_val == 'write') {
      Logger.log(JSON.stringify(rowDataLog));
      
      sheet_target.insertRows(2);
      var newRangeDataLog = sheet_target.getRange(2, 1, 1, rowDataLog.length);
      newRangeDataLog.setValues([rowDataLog]);
      
      maxRowData(10, sheet_id);
      
      return ContentService.createTextOutput(result);
    }
    
    if (sts_val == 'read') {
      var lastData = sheet_target.getRange(2, 1, 1, 4).getDisplayValues();
      return ContentService.createTextOutput(JSON.stringify(lastData));
    }
  }
  
  return ContentService.createTextOutput(result);
}

function maxRowData(maxRows, sheet_id) {
  var sheet = SpreadsheetApp.openById(sheet_id).getSheetByName('DATA');
  
  var lastRow = sheet.getLastRow();
  if (lastRow > maxRows) {
    sheet.deleteRows(maxRows + 1, lastRow - maxRows);
  }
}

function stripQuotes(value) {
  return value.replace(/^["']|['"]$/g, "");
}

function testDoGet() {
  var e = {
    parameter: {
      sts: 'write',
      hr: '75.5',
      spo2: '98'
    }
  };
  var result = doGet(e);
  Logger.log(result.getContent());
}
