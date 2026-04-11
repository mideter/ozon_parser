"""Модуль для поиска элементов товаров на странице (как в ozon_radar/ozon_py)."""
from typing import List

from selenium.webdriver.common.by import By


class ElementFinder:
    """Класс для поиска элементов товаров на странице."""

    def __init__(self, driver):
        self.driver = driver

    def find_product_elements(self) -> List:
        product_elements = []

        try:
            tile_grids = self.driver.find_elements(
                By.CSS_SELECTOR,
                'div[data-widget="tileGridDesktop"], div[data-widget="tileGridMobile"]',
            )

            for grid in tile_grids:
                try:
                    products = grid.find_elements(
                        By.CSS_SELECTOR, "div.tile-root[data-index]"
                    )
                    product_elements.extend(products)
                except Exception:
                    continue

        except Exception:
            try:
                product_elements = self.driver.find_elements(
                    By.CSS_SELECTOR, "div.tile-root[data-index]"
                )
            except Exception:
                product_elements = []

        return product_elements

