//
//  graphView.swift
//  SenStickViewer
//
//  Created by AkihiroUehara on 2016/05/24.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import UIKit
import SenStickSDK

struct graphData {
    var color    :UIColor
    var data     :[Double]
    var maxValue :Double
    var minValue :Double
    var drawArea :Double
}

class DataGraphView : UIView {
    var graphDataArray : [graphData]?
    
    internal func draw(data : [graphData]) {
        self.graphDataArray = data
        setNeedsDisplay()
    }
    
    override func drawRect(rect: CGRect) {
        super.drawRect(rect)
        
        if graphDataArray == nil {
            return
        }
        
        let context = UIGraphicsGetCurrentContext()!
        
        for graphData in graphDataArray! {
            // 描画範囲の幅と高さ
            let height = rect.size.height
            let width  = rect.size.width * CGFloat(graphData.drawArea)
            // 最大/最小値で
            let dx     = width  / CGFloat(graphData.data.count)
            let scaleY = height / CGFloat(graphData.maxValue - graphData.minValue)
            let offsetY = -1 * scaleY * CGFloat(graphData.minValue)
            
            let path = drawPath(graphData.data, dx: dx, scaleY: scaleY, offsetY: offsetY)
            
            CGContextAddPath(context, path)
            CGContextSetLineWidth(context, 2)
            CGContextSetStrokeColorWithColor(context, graphData.color.CGColor)
            CGContextStrokePath(context)
        }
    }
    
    func drawPath(data: [Double], dx: CGFloat, scaleY: CGFloat, offsetY: CGFloat) -> CGPath
    {
        let path = CGPathCreateMutable()
        guard data.count > 1 else {
            return path
        }
        
        // 開始点
        CGPathMoveToPoint(path, nil, 0, CGFloat(data[0]) * scaleY + offsetY)
        var x: CGFloat = 0
        for d in data {
            CGPathAddLineToPoint(path, nil, x, CGFloat(d) * scaleY + offsetY)
            x = x * dx
        }
        
        return path
    }
}
