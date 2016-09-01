//: Playground - noun: a place where people can play

import Cocoa

let names = ["n1", "n2", "n3"]
let str = names.reduce("") {(s, item) -> String in s + "," + item}
"\(str)"

