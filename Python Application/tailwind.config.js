/** @type {import('tailwindcss').Config} */
module.exports = {
  darkMode: 'class',
  content: ["./templates/*.html",
            "./static/src/**/*.js",
            "./node_modules/flowbite/**/*.js"
  ],
  theme: {
    extend: {},
    fontFamily: {
      grotesk: ['Space Grotesk', 'sans-serif'],
      sans: ['Poppins', 'sans-serif'],
    },
    colors: {
      'darkb': '#00001c',
      'frost': '#fcfbfc',
      'rice': '#fbf5ef',
    }
    
  },
  plugins: [
    require("flowbite/plugin")
  ],
}



