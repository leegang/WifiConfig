const path = require('path');
const glob = require('glob-all');
const webpack = require('webpack');
const { merge } = require('webpack-merge');

const { PurgeCSSPlugin } = require('purgecss-webpack-plugin');

const baseWebpackConfig = require('./webpack.base');

module.exports = merge(baseWebpackConfig, {
    mode: 'production',
    devtool: false,
    optimization: {
        moduleIds: 'deterministic',
        splitChunks: {
            chunks: 'all',
        },
    },
    plugins: [
        new webpack.DefinePlugin({
            'process.env.NODE_ENV': JSON.stringify('production'),
        }),
        new PurgeCSSPlugin({
            paths: glob.sync([
                path.join(__dirname, 'index.html'),
                path.join(__dirname, '**/*.vue'),
                path.join(__dirname, '**/*.js'),
            ]),
            safelist: [
                /-(leave|enter|appear)(|-(to|from|active))$/,
                /^(?!(|.*?:)cursor-move).+-move$/,
                /^router-link(|-exact)-active$/,
                /data-v-.*/,
            ],
        }),
    ],
});