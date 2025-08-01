const path = require('path');
const webpack = require('webpack');
const { merge } = require('webpack-merge');

const baseWebpackConfig = require('./webpack.base');

module.exports = merge(baseWebpackConfig, {
    mode: 'development',
    devtool: 'source-map',
    devServer: {
        port: 8080,
        static: {
            directory: path.join(__dirname, '../data'),
        },
        compress: true,
        hot: true,
        historyApiFallback: true,
        allowedHosts: 'all',
        headers: {
            'Access-Control-Allow-Origin': '*',
            'Access-Control-Allow-Methods': 'GET, POST, PUT, DELETE, PATCH, OPTIONS',
            'Access-Control-Allow-Headers': 'X-Requested-With, content-type, Authorization'
        }
    },
    plugins: [
        new webpack.DefinePlugin({
            'process.env.NODE_ENV': JSON.stringify('development')
        }),
    ],
});
