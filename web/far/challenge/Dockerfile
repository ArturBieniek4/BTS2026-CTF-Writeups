FROM composer:2.8.8 AS composer
FROM php:7.4.33-apache-bullseye

RUN apt-get update -y && \
    apt-get install -y git zlib1g-dev libpng-dev openssl libssl-dev libpcap0.8

COPY --from=composer /usr/bin/composer /usr/local/bin/composer

WORKDIR /var/www/html

RUN docker-php-ext-install gd
RUN composer require mpdf/mpdf:8.2.6
RUN composer require fakerphp/faker:1.12.0
RUN composer require symfony/process:4.1

COPY app/* /var/www/html/
COPY app/api /var/www/html/api
COPY app/uploads /var/www/html/uploads
RUN chown -R www-data:www-data /var/www/html/uploads && \
    chmod -R 755 /var/www/html/uploads

RUN printf '%s\n' \
    '<Directory "/var/www/html/uploads">' \
    '    php_admin_flag engine off' \
    '    <FilesMatch "\\.ph(p[0-9]?|tml)$">' \
    '        Require all denied' \
    '    </FilesMatch>' \
    '</Directory>' \
    > /etc/apache2/conf-available/uploads-no-php.conf && \
    a2enconf uploads-no-php

RUN echo "BtSCTF{fake_flag}" > /flag.txt && \
    chmod 444 /flag.txt

EXPOSE 80
CMD ["apache2-foreground"]