select avg(c.level) as 'average_level'
from CatchedPokemon c
where c.owner_id in ( select id
     from Trainer
     where name = 'Red'
    );