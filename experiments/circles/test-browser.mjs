// Optional interaction regressions: requires Playwright and a Chromium install.
import assert from "node:assert/strict";
import http from "node:http";
import fs from "node:fs/promises";
import path from "node:path";
import { fileURLToPath } from "node:url";
const { chromium } = await import(
  process.env.PLAYWRIGHT_MODULE ?? "playwright"
);
const root = fileURLToPath(new URL("../../", import.meta.url));
const server = http.createServer(async (req, res) => {
  try {
    let name = new URL(req.url, "http://localhost").pathname;
    if (name.endsWith("/")) name += "index.html";
    const file = path.resolve(root, "." + decodeURIComponent(name));
    if (!file.startsWith(root)) throw Error("Outside test root");
    res.setHeader(
      "Content-Type",
      file.endsWith(".mjs")
        ? "text/javascript"
        : file.endsWith(".css")
          ? "text/css"
          : "text/html",
    );
    res.end(await fs.readFile(file));
  } catch {
    res.statusCode = 404;
    res.end();
  }
});
await new Promise((resolve) => server.listen(0, "127.0.0.1", resolve));
let browser;
try {
  browser = await chromium.launch({
    headless: true,
    ...(process.env.CHROMIUM_PATH
      ? { executablePath: process.env.CHROMIUM_PATH }
      : {}),
  });
  const page = await browser.newPage({
    viewport: { width: 1440, height: 1000 },
  });
  const errors = [];
  page.on("pageerror", (error) => errors.push(error.message));
  await page.goto(
    `http://127.0.0.1:${server.address().port}/experiments/circles/`,
  );
  const click = (name) =>
    page.getByRole("button", { name, exact: true }).click();
  const text = (id) => page.locator(`#${id}`).textContent();

  // A mouse-selected preset retains button focus. Letter shortcuts must work.
  await click("Two independent pops");
  await page.keyboard.press("j");
  assert.match(await text("selection"), /Selected mul at 2/);
  await page.keyboard.press("k");
  assert.match(await text("selection"), /Selected mul at 1/);
  await page.locator("#pop").click();
  await page.keyboard.press("p");
  assert.equal(await text("remaining"), "1");
  await page.keyboard.press("p");
  assert.equal(await text("expression"), "26");
  await page.keyboard.press("u");
  assert.equal(await text("remaining"), "1");
  await page.locator("#undo").click();
  await page.keyboard.press("r");
  assert.equal(await text("pops"), "0");
  // Native Enter on a preset activates it, rather than popping a circle.
  await page
    .getByRole("button", { name: "Two independent pops", exact: true })
    .focus();
  await page.keyboard.press("Enter");
  assert.equal(await text("pops"), "0");
  console.log(
    "PASS letter shortcuts after mouse clicks; native button activation",
  );

  await click("A symbol waits");
  const field = page.getByLabel("Integer value for x");
  await field.fill("5");
  await page
    .getByRole("button", { name: "Select ready mul at 2", exact: true })
    .click();
  assert.equal(await field.inputValue(), "5");
  await page.locator("#compare").click();
  assert.equal(await field.inputValue(), "5");
  await page.locator("#pop").click();
  assert.equal(await field.inputValue(), "5");
  await field.focus();
  await page.keyboard.press("j");
  assert.equal(await field.inputValue(), "5j");
  await field.fill("5");
  await click("Substitute");
  await page.locator("#undo").click();
  assert.equal(await field.inputValue(), "5");
  await page.locator("#reset").click();
  assert.equal(await field.inputValue(), "");
  await field.fill("5");
  await click("Substitute");
  await page.locator("#parallel").click();
  await page.locator("#parallel").click();
  assert.equal(await text("expression"), "17");
  console.log(
    "PASS substitution drafts survive selection, comparison, pops and undo; reset clears drafts; typing stays local",
  );

  await click("Seven circles, three rounds");
  for (let i = 0; i < 3; i++) await page.locator("#parallel").click();
  assert.equal(await text("expression"), "8");
  assert.equal(await text("rounds"), "3");
  await click("Recolour, then rotate");
  await page.locator("#pop").click();
  assert.match(await text("expression"), /image\(red \. red\/\. red \.\)/);
  await page.locator("#pop").click();
  assert.equal(await page.locator(".pixel").count(), 6);
  const pending = page.waitForEvent("download");
  await page.locator("#export").click();
  const download = await pending;
  const trace = JSON.parse(await fs.readFile(await download.path(), "utf8"));
  assert.equal(trace.final.kind, "image");
  assert.equal(trace.actions.length, 2);
  await page.setViewportSize({ width: 390, height: 844 });
  await click("Seven circles, three rounds");
  assert.equal(
    await page.evaluate(
      () => document.documentElement.scrollWidth > innerWidth,
    ),
    false,
  );
  await page.locator("summary").click();
  await page.locator("#source").fill('{"kind":"num","value":true}');
  await click("Load expression");
  assert.match(await text("message"), /decimal integer strings/);
  assert.deepEqual(errors, []);
  console.log(
    "PASS parallel rounds, separated pixel names, image rendering, exact trace export, mobile containment and invalid input",
  );
} finally {
  await browser?.close();
  await new Promise((resolve) => server.close(resolve));
}
