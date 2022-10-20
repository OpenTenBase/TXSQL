const SM3 = require('../src/sm3');


test('test sm3 performance', () => {
  const sm3 = new SM3();
  const begin = new Date().getTime();
  for (let i = 0; i < 1000; i++) {
    // eslint-disable-next-line no-unused-vars
    const result = sm3.hashForUTF8String('This website stores cookies on your computer. These cookies are used to collect information about how you interact with our website and allow us to remember you. We use this information in order to improve and customize your browsing experience and for analytics and metrics about our visitors both on this website and other media. To find out more about the cookies we use, see our Privacy Policy.');
  }
  const end = new Date().getTime();
  const diff = end - begin;
  const performance = 1000.0 / (diff / 1000.0);
  console.log(performance);
});
