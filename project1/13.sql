select p.name, p.id
from CatchedPokemon c, Pokemon p
where c.pid = p.id
       and c.owner_id in(select id
                    from Trainer
                    where hometown = 'Sangnok City')
order by p.id;                    