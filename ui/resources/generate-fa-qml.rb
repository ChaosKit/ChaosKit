#!/usr/bin/env ruby
if ARGV.length < 2
  $stderr.puts 'Usage: generate-fa-qml.rb path/to/icons.json path/to/icons.qml'
  exit false
end

require 'active_support/core_ext/string'
require 'json'

required_styles = Set['regular', 'solid']
json_path, qml_path = ARGV

File.open(qml_path, 'w') do |qml_file|
  json = JSON.parse! File.read(json_path)

  qml_file.puts <<~QML
  pragma Singleton
  import QtQuick 2.0

  QtObject {
  QML

  json.each_pair do |icon, metadata|
    next unless required_styles.intersect? Set.new(metadata['styles'])

    translated_name = 'fa' + icon.tr('-', '_').camelize
    unicode = metadata['unicode']

    qml_file.puts %Q(  readonly property string #{translated_name}: "\\u#{unicode}")
  end

  qml_file.puts '}'
end
