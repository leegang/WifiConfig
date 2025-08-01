module.exports = {
    content: [
        './src-js/**/*.html',
        './src-js/**/*.vue',
        './src-js/**/*.jsx',
        './src-js/**/*.js',
    ],
    theme: {
        extend: {
            colors: {
                main: {
                    DEFAULT: '#656d9a',
                    dark: '#36374c',
                    light: '#a4abc5'
                },
                primary: {
                    DEFAULT: '#7680ff',
                    light: 'rgba(118,128,255,.1)',
                },
                secondary: {
                    DEFAULT: '#2ddab5',
                    light: 'rgba(45,218,181,.08)',
                }
            }
        }
    },
    plugins: [],
}