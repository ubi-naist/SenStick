//
//  logListViewController.swift
//  SenStickViewer
//
//  Created by AkihiroUehara on 2016/06/26.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import UIKit
import SenStickSDK

class LogListViewController: UITableViewController, SenStickDeviceDelegate, SenStickMetaDataServiceDelegate {

    var device: SenStickDevice?

    // 画面遷移先のセンサのログデータ出力で、記録開始時刻が必要になる。その時刻があるセンサのメタデータを、この辞書で保持する。
    var sensorMetaData:Dictionary<Int,SenStickMetaDataService> = [:]
    
    // View Controller life cycle
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        
        self.device?.delegate = self
        self.device?.metaDataService?.delegate = self
    }
    
    // SenStickDeviceDelegate
    func didServiceFound(_ sender: SenStickSDK.SenStickDevice)
    {
    }
    func didConnected(_ sender: SenStickSDK.SenStickDevice)
    {
    }
    func didDisconnected(_ sender: SenStickSDK.SenStickDevice)
    {
        _ = self.navigationController?.popToRootViewController(animated: true)
    }
    
    // SenStickSensorServiceDelegate
    func didUpdateMetaData(_ sender:SenStickMetaDataService)
    {
        if let cell = self.tableView.cellForRow(at: IndexPath(row: Int(sender.logID), section: 0)) {
            // センサメタデータを辞書に保存する。
            sensorMetaData[Int(sender.logID)] = sender
            
            cell.textLabel?.text = "Log ID:\(sender.logID)"

            let formatter = DateFormatter()
            formatter.dateFormat = "  yyyy年MM月dd日 HH時mm分ss秒 Z"
            cell.detailTextLabel?.text = formatter.string(from: sender.dateTime)
        }
    }
    
    // segue
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        if let vc = segue.destination as? LogReaderViewController {
            vc.device = self.device
            vc.logID  = UInt8((self.tableView.indexPathForSelectedRow! as NSIndexPath).row)
            vc.sensorMetaData = self.sensorMetaData[(self.tableView.indexPathForSelectedRow! as NSIndexPath).row]
        }
    }
    
    // MARK: - Table View
    override func numberOfSections(in tableView: UITableView) -> Int
    {
        return 1
    }
    
    override func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int
    {
        if device?.controlService?.command == .stopping {
            return Int(device!.controlService!.availableLogCount)
        } else {
            return max(0, Int(device!.controlService!.availableLogCount) - 1)
        }
    }
    
    override func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let cell = tableView.dequeueReusableCell(withIdentifier: "logListCell", for: indexPath)
        cell.textLabel?.text = "Log ID:\((indexPath as NSIndexPath).row)"
        
        device?.metaDataService?.requestMetaData(UInt8(indexPath.row))
        
        return cell
    }
}
