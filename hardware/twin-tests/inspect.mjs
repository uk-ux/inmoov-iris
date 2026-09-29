import {chromium} from 'playwright-core';
const browser=await chromium.launch({executablePath:'C:/Program Files (x86)/Microsoft/Edge/Application/msedge.exe',headless:true,args:['--use-angle=swiftshader','--enable-unsafe-swiftshader']});
const page=await browser.newPage({viewport:{width:1450,height:1100}});await page.goto('http://127.0.0.1:8765/twin/inspect.html');await page.waitForFunction(()=>window.inspect);await page.evaluate(n=>window.inspect(n),process.argv[2]||'TopskullV');await page.screenshot({path:'U:/inmoov/hardware/component-inspect.png',fullPage:true});await browser.close();
