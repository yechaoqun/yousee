const path = require('path');
const webpack = require('webpack');
const BundleAnalyzerPlugin = require('webpack-bundle-analyzer').BundleAnalyzerPlugin;
const ManifestPlugin = require('webpack-manifest-plugin');

module.exports = {
    entry: {
    	main:["./src/main.js"],
    	room:["./src/room.js"]
    },
    output: {
        path: path.resolve(__dirname, 'dist'),
        filename: "[name].js"
    },
    module: {
        rules:[
            { test: /\.css$/, use: ['css-loader'] },
            { test: /(\.jsx|\.js)$/, 
            	exclude: /(node_modules|bower_components)/,
            	use: {
				        loader: 'babel-loader',
				        options: {
                          presets: ['@babel/preset-env', '@babel/preset-react'],
                          plugins: [
                            ["babel-plugin-styled-components", {"displayName": false}]
                          ]
				        }
				      }
            }
        ]
   },
   optimization: {
        splitChunks: {
        // include all types of chunks
        chunks: 'all'
        },

        runtimeChunk: true
    },

    
}
