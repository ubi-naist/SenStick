//
//  SensorDataViewController.swift
//  SenStickViewer
//
//  Created by AkihiroUehara on 2016/05/22.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import UIKit
import SenStickSDK

class SensorDataViewController : UITableViewController {
    weak var device: SenStickDevice?
    
    override func viewDidLoad() {
        super.viewDidLoad()
        self.tableView.delegate = self
    }
    
    override func viewWillAppear(animated: Bool) {
        super.viewWillAppear(animated)

        device?.connect()
    }
    
    override func viewWillDisappear(animated: Bool) {
        super.viewWillDisappear(animated)
    }
    
    override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?) {
    }
    
    // MARK: - KVO
    
    // MARK: - Table View
    override func numberOfSectionsInTableView(tableView: UITableView) -> Int {
        return 1
    }

    override func tableView(tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return 2
    }

    override func tableView(tableView: UITableView, cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {
        if indexPath.row == 0 {
            return tableView.dequeueReusableCellWithIdentifier("statusCell", forIndexPath: indexPath)
        } else {
            return tableView.dequeueReusableCellWithIdentifier("dataCell", forIndexPath: indexPath)
        }
    }
    
    override func tableView(tableView: UITableView, heightForRowAtIndexPath indexPath: NSIndexPath) -> CGFloat {
        return 148
    }

    override func tableView(tableView: UITableView, willDisplayCell cell: UITableViewCell, forRowAtIndexPath indexPath: NSIndexPath) {
        if let statusCell = cell as? SensorStatusCellView {
            statusCell.device = self.device
        }
    }
}
