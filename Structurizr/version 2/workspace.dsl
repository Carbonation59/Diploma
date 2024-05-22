workspace {

    model {
        user = person "User"
        db = softwareSystem "Database"
        gen = softwareSystem "Generator"
        main = softwareSystem "Main Algorithm"

        user -> main "Запуск алгоритма"
        main -> gen "Вызов генератора (если требуется)"
        gen -> db "Генерация данных"
        main -> db "Извлечение данных"
    }

    views {
        systemLandscape {
            include *
            autolayout
        }
        terminology {

        }
        themes default
    }
    
}